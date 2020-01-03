#include <AB/CommonConfig.h>
#include "Database.h"
#include "FileUtils.h"
#include "Logger.h"
#include "Logo.h"
#include "SimpleConfigManager.h"
#include "SqlReader.h"
#include "StringUtils.h"
#include "Utils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sa/ArgParser.h>
#include <streambuf>
#include "UuidUtils.h"

namespace fs = std::filesystem;

static bool sReadOnly = false;
static bool sVerbose = false;

static void ShowLogo()
{
    std::cout << "This is AB Database tool" << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-" << CURRENT_YEAR << std::endl;

    std::cout << std::endl;

    std::cout << AB_CONSOLE_LOGO << std::endl;

    std::cout << std::endl;
}

static void ShowHelp(const sa::arg_parser::cli& _cli)
{
    std::cout << std::endl;
    std::cout << sa::arg_parser::get_help("dbtool", _cli);
    std::cout << std::endl;
    std::cout << "ACTIONS" << std::endl;
    std::cout << "    update: Update the database" << std::endl;
    std::cout << "    versions: Show database and table versions" << std::endl;
    std::cout << "    acckeys: Show account keys" << std::endl;
    std::cout << "    genacckey: Generate an account key" << std::endl;
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

    cli.push_back({ "action", { "-a", "--action" }, "What to do, possible value(s) see bellow",
        true, true, sa::arg_parser::option_type::string });
    cli.push_back({ "readonly", { "-r", "--read-only" }, "Do not write to Database",
        false, false, sa::arg_parser::option_type::none });
    cli.push_back({ "verbose", { "-v", "--verbose" }, "Write out stuff",
        false, false, sa::arg_parser::option_type::none });
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
    cli.push_back({ "schemadir", { "-d", "--schema-dir" }, "Directory with .sql files to import",
        false, true, sa::arg_parser::option_type::string });
}

static void GetFiles(const std::string& dir, std::map<unsigned, std::string>& result)
{
    for (const auto& entry : fs::directory_iterator(dir))
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

    SqlReader reader;
    if (!reader.Read(file))
    {
        std::cerr << "Error reading file " << file << std::endl;
        return false;
    }
    if (reader.IsEmpty())
    {
        std::cout << "File " << file << " is empty" << std::endl;
        // I guess empty files are okay.
        return true;
    }

    DB::DBTransaction transaction(&db);
    if (!transaction.Begin())
        return false;

    bool failed = false;
    reader.VisitStatements([&](const auto& statement) -> Iteration
    {
        if (sVerbose)
            std::cout << statement << std::endl;
        if (!sReadOnly)
        {
            if (!db.ExecuteQuery(statement))
            {
                failed = true;
                return Iteration::Break;
            }
        }
        return Iteration::Continue;
    });

    if (failed)
        return false;

    // End transaction
    return transaction.Commit();
}

static int GetDBVersion(DB::Database& db)
{
    std::ostringstream query;
#ifdef USE_PGSQL
    if (DB::Database::driver_ == "pgsql")
        query << "SET search_path TO schema, public; ";
#endif
    query << "SELECT `value` FROM `versions` WHERE ";
    query << "`name` = " << db.EscapeString("schema");

    std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str());
    if (!result)
    {
        // No such record means not even the first file (file 0) was imported
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
        // Files must update the version, so with each import this value should change
        int version = GetDBVersion(db);
        if (sVerbose)
            std::cout << "Database version is: " << version << std::endl;
        if (static_cast<int>(f.first) > version)
        {
            if (!ImportFile(db, f.second))
                return false;
        }
    }

    std::cout << "Database version is now: " << GetDBVersion(db) << std::endl;

    return true;
}

static void ShowVersions(DB::Database& db)
{
    std::ostringstream query;
    query << "SELECT * FROM `versions` ORDER BY `name`";
    for (std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str()); result; result = result->Next())
    {
        std::cout << result->GetString("name");
        std::cout << '\t';
        std::cout << result->GetUInt("value");
        std::cout << std::endl;
    }
}

static void ShowAccountKeys(DB::Database& db)
{
    std::ostringstream query;
    query << "SELECT * FROM `account_keys`";
    for (std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str()); result; result = result->Next())
    {
        std::cout << result->GetString("uuid");
        std::cout << '\t';
        std::cout << result->GetUInt("used") << "/" << result->GetUInt("total");
        std::cout << '\t';
        std::cout << result->GetString("description");
        std::cout << std::endl;
    }
}

static std::string GenAccKey(DB::Database& db)
{
    std::string uuid = Utils::Uuid::New();
    std::ostringstream query;
    query << "INSERT INTO `account_keys` (`uuid`, `used`, `total`, `description`, `status`, `key_type`";
    query << ") VALUES (";
    query << db.EscapeString(uuid) << ",";
    query << "0,";
    query << "1,";
    query << db.EscapeString("Generated by dbtool") << ",";
    query << "2,";
    query << "1";
    query << ")";

    DB::DBTransaction transaction(&db);
    if (!transaction.Begin())
        return "Error creating transaction";

    if (!sReadOnly)
    {
        if (!db.ExecuteQuery(query.str()))
            return "Error";
    }

    // End transaction
    if (!transaction.Commit())
        return "Error";

    return uuid;
}

/// What should we do.
/// At the moment we can only update the DB
enum class Action
{
    Update,
    Versions,
    AccountKeys,
    GenAccKey,
};

int main(int argc, char** argv)
{
    ShowLogo();

    const std::string exeFile = Utils::GetExeName();
    const std::string path = Utils::ExtractFileDir(exeFile);
    std::string schemasDir = path + "/../sql";

    sa::arg_parser::cli _cli{ {
        { "help", { "-h", "-help", "-?" }, "Show help", false, false, sa::arg_parser::option_type::none }
    } };
    InitCli(_cli);

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
        action = Action::Update;
    else if (sActval.compare("versions") == 0)
        action = Action::Versions;
    else if (sActval.compare("acckeys") == 0)
        action = Action::AccountKeys;
    else if (sActval.compare("genacckey") == 0)
        action = Action::GenAccKey;
    else
    {
        std::cerr << "Unknown action " << sActval << std::endl;
        ShowHelp(_cli);
        return EXIT_FAILURE;
    }
    sReadOnly = sa::arg_parser::get_value<bool>(parsedArgs, "readonly", false);
    sVerbose = sa::arg_parser::get_value<bool>(parsedArgs, "verbose", false);

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

    std::unique_ptr<DB::Database> db = std::unique_ptr<DB::Database>(DB::Database::CreateInstance(
        DB::Database::driver_,
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

    if (sReadOnly)
        std::cout << "READ-ONLY mode, nothing is written to the database" << std::endl;

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
    else if (action == Action::Versions)
    {
        ShowVersions(*db);
    }
    else if (action == Action::AccountKeys)
    {
        ShowAccountKeys(*db);
    }
    else if (action == Action::GenAccKey)
    {
        std::string key = GenAccKey(*db);
        std::cout << key << std::endl;
    }
    else
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
