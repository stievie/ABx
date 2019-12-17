#include <iostream>
#include "SimpleConfigManager.h"
#include <sa/ArgParser.h>
#include "Logo.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "Utils.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "Database.h"
#include <memory>
#include "Logger.h"
#include <filesystem>
#include <map>
#include <fstream>
#include <streambuf>

static void ShowLogo()
{
    std::cout << "This is AB Database tool" << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-2019" << std::endl;

    std::cout << std::endl;

    std::cout << AB_CONSOLE_LOGO << std::endl;

    std::cout << std::endl;
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << std::endl;
    std::cout << sa::arg_parser::get_help("dbtool", _cli);
}

static void InitCli(sa::arg_parser::cli& cli)
{
    std::stringstream dbDrivers;
#ifdef USE_SQLITE
    dbDrivers << " sqlite";
#endif
#ifdef USE_MYSQL
    dbDrivers << " mysql";
#endif
#ifdef USE_PGSQL
    dbDrivers << " pgsql";
#endif
#ifdef USE_ODBC
    dbDrivers << " odbc";
#endif

    cli.push_back({ "action", { "-a", "--action" }, "What to do, possible value(s): update",
        true, true, sa::arg_parser::option_type::string });
    cli.push_back({ "dbdriver", { "-dbdriver", "--database-driver" }, "Database driver, possible value(s):" + dbDrivers.str(),
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "dbhost", { "-dbhost", "--database-host" }, "Host name of database server",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "dbport", { "-dbport", "--database-port" }, "Port to connect to",
        false, true, sa::arg_parser::option_type::integer });
    cli.push_back({ "dbname", { "-dbname", "--database-name" }, "Name of database",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "dbuser", { "-dbuser", "--database-user" }, "User name for database",
        false, true, sa::arg_parser::option_type::string });
    cli.push_back({ "dbpass", { "-dbpass", "--database-password" }, "Password for database",
        false, true, sa::arg_parser::option_type::string });
}

static void GetFiles(const std::string& dir, std::map<unsigned, std::string>& result)
{
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        const auto path = entry.path().string();
        const std::string filename = Utils::ExtractFileName(path);
        unsigned int version;
#ifdef _MSC_VER
        if (sscanf_s(filename.c_str(), "schema.%u.sql", &version) != 1)
            continue;
#else
        if (sscanf(filename.c_str(), "schema.%u.sql", &version) != 1)
            continue;
#endif
        result[version] = path;
    }
}

static bool ImportFile(DB::Database& db, const std::string& file)
{
    std::cout << "Importing " << file << std::endl;

    std::ifstream t(file);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
    if (str.empty())
    {
        std::cout << "File " << file << " is empty" << std::endl;
        return true;
    }

    DB::DBTransaction transaction(&db);
    if (!transaction.Begin())
        return false;

    if (db.ExecuteQuery(str))
        return false;

    // End transaction
    return transaction.Commit();

}

static int GetDBVersion(DB::Database& db)
{
    std::ostringstream query;
    query << "SELECT * FROM `versions` WHERE ";
    query << "`name` = " << db.EscapeString("schema");

    std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str());
    if (!result)
    {
        std::cerr << "Unable to read DB schema version" << std::endl;
        return -1;
    }

    return static_cast<int>(result->GetUInt("value"));
}

static bool UpdateDatabase(DB::Database& db, const std::string& dir)
{
    std::cout << "Importing directory " << dir << std::endl;

    std::map<unsigned, std::string> files;
    GetFiles(dir, files);
    // Fortunately the items in a map are sorted by key
    for (const auto& f : files)
    {
        int version = GetDBVersion(db);
        if (version == -1)
            return false;
        if (f.first > static_cast<unsigned>(version))
        {
            if (!ImportFile(db, f.second))
                return false;
        }
    }

    return true;
}

enum class Action
{
    Update
};

int main(int argc, char** argv)
{
    ShowLogo();

    std::string exeFile;
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileNameA(NULL, buff, MAX_PATH);
    exeFile = std::string(buff);
#else
    char buff[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buff, PATH_MAX);
    exeFile = std::string(buff, (count > 0) ? count : 0);
#endif
    std::string path = Utils::ExtractFileDir(exeFile);
    std::string schemasDir = path + "/../sql";

    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none }
    } };
    InitCli(_cli);
    _cli.push_back({ "schemadir", { "-d", "--schema-dir" }, "Directory with .sql files to import",
        false, true, sa::arg_parser::option_type::string });

    sa::arg_parser::values parsedArgs;
    sa::arg_parser::result cmdres = sa::arg_parser::parse(argc, argv, _cli, parsedArgs);
    auto val = sa::arg_parser::get_value<bool>(parsedArgs, "help");
    if (val.has_value() && val.value())
    {
        ShowHelp(_cli);
        return EXIT_SUCCESS;
    }
    if (!cmdres)
    {
        std::cout << cmdres << std::endl;
        std::cout << "Type `dbtool -h` for help." << std::endl;
        return EXIT_FAILURE;
    }

    auto actval = sa::arg_parser::get_value<std::string>(parsedArgs, "action");
    if (!actval.has_value())
    {
        std::cerr << "No action provided" << std::endl;
        ShowHelp(_cli);
        return EXIT_FAILURE;
    }
    const std::string& sActval = actval.value();
    Action action;
    if (sActval.compare("update") == 0)
    {
        action = Action::Update;
    }
    else
    {
        std::cerr << "Unknown action " << sActval << std::endl;
        ShowHelp(_cli);
        return EXIT_FAILURE;
    }

    std::string cfgFile = path + "/config/db.lua";
    IO::SimpleConfigManager cfg;
    if (!cfg.Load(cfgFile))
    {
        std::cerr << "Failed to load config file " << cfgFile << std::endl;
        return EXIT_FAILURE;
    }

    DB::Database::driver_ = sa::arg_parser::get_value<std::string>(parsedArgs, "dbdriver", DB::Database::driver_);
    DB::Database::dbHost_ = sa::arg_parser::get_value<std::string>(parsedArgs, "dbhost", DB::Database::dbHost_);
    DB::Database::dbName_ = sa::arg_parser::get_value<std::string>(parsedArgs, "dbname", DB::Database::dbName_);
    DB::Database::dbUser_ = sa::arg_parser::get_value<std::string>(parsedArgs, "dbuser", DB::Database::dbUser_);
    DB::Database::dbPass_ = sa::arg_parser::get_value<std::string>(parsedArgs, "dbpass", DB::Database::dbPass_);
    DB::Database::dbPort_ = sa::arg_parser::get_value<uint16_t>(parsedArgs, "dbport", DB::Database::dbPort_);

    if (DB::Database::driver_.empty())
        DB::Database::driver_ = cfg.GetGlobalString("db_driver", "");
    if (DB::Database::dbHost_.empty())
        DB::Database::dbHost_ = cfg.GetGlobalString("db_host", "");
    if (DB::Database::dbName_.empty())
        DB::Database::dbName_ = cfg.GetGlobalString("db_name", "");
    if (DB::Database::dbUser_.empty())
        DB::Database::dbUser_ = cfg.GetGlobalString("db_user", "");
    if (DB::Database::dbPass_.empty())
        DB::Database::dbPass_ = cfg.GetGlobalString("db_pass", "");
    if (DB::Database::dbPort_ == 0)
        DB::Database::dbPort_ = static_cast<uint16_t>(cfg.GetGlobalInt("db_port", 0ll));

    std::unique_ptr<DB::Database> db = std::unique_ptr<DB::Database>(DB::Database::CreateInstance(DB::Database::driver_,
        DB::Database::dbHost_, DB::Database::dbPort_,
        DB::Database::dbUser_, DB::Database::dbPass_,
        DB::Database::dbName_
    ));
    if (!db || !db->IsConnected())
    {
        std::cerr << "Database connection failed" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Connected to " << DB::Database::driver_ <<
        " " << DB::Database::dbUser_ << "@" << DB::Database::dbHost_ << ":" << DB::Database::dbPort_ << std::endl;

    if (action == Action::Update)
    {
        schemasDir = sa::arg_parser::get_value<std::string>(parsedArgs, "schemadir", schemasDir);
        if (schemasDir.empty())
        {
            std::cerr << "Schema directory is empty" << std::endl;
            return EXIT_FAILURE;
        }
        if (!UpdateDatabase(*db, schemasDir))
            return EXIT_FAILURE;
    }
    else
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
