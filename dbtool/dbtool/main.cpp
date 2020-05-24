/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sa/Compiler.h>
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4702 4127)
#   include <lua.hpp>
#   include <kaguya/kaguya.hpp>
PRAGMA_WARNING_POP
#include "SqlReader.h"
#include <AB/CommonConfig.h>
#include <abdb/Database.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Logger.h>
#include <abscommon/Logo.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Utils.h>
#include <abscommon/UuidUtils.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sa/ArgParser.h>
#include <sa/table.h>
#include <streambuf>
#include "LuaSkill.h"


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
    sa::tab::table table;
    table << "    update" << sa::tab::endc << "Update the database" << sa::tab::endr;
    table << "    versions" << sa::tab::endc << "Show database and table versions" << sa::tab::endr;
    table << "    acckeys" << sa::tab::endc << "Show account keys" << sa::tab::endr;
    table << "    genacckey" << sa::tab::endc << "Generate a new account key" << sa::tab::endr;
    table << "    updateskills" << sa::tab::endc << "Update skills stats in DB" << sa::tab::endr;
    std::cout << table;
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
    cli.push_back({ "schemadir", { "-d", "--schema-dir" }, "Directory with .sql files to import for updating",
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

    size_t importedCount = 0;
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
            ++importedCount;
            if (!ImportFile(db, f.second))
                return false;
        }
    }

    std::cout << "Imported " << importedCount << " files" << std::endl;
    std::cout << "Database version is now: " << GetDBVersion(db) << std::endl;

    return true;
}

static void ShowVersions(DB::Database& db)
{
    std::ostringstream query;
    query << "SELECT * FROM `versions` ORDER BY `name`";
    sa::tab::table table;
    table.table_sep_ = '=';
    table << sa::tab::head << "Name" << sa::tab::endc << "Value" << sa::tab::endr;
    for (std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str()); result; result = result->Next())
    {
        table << result->GetString("name") << sa::tab::endc <<
                 sa::tab::ralign << result->GetUInt("value") << sa::tab::endr;
    }
    std::cout << table;
}

static void ShowAccountKeys(DB::Database& db)
{
    std::ostringstream query;
    query << "SELECT * FROM `account_keys`";
    sa::tab::table table;
    table.table_sep_ = '=';
    table.col_sep_ = " | ";
    table << sa::tab::head << "UUID" << sa::tab::endc << sa::tab::ralign << "Used" <<
             sa::tab::endc << sa::tab::ralign << "Total" << sa::tab::endc << "Description" << sa::tab::endr;
    for (std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str()); result; result = result->Next())
    {
        table << result->GetString("uuid") << sa::tab::endc <<
                 sa::tab::ralign << result->GetUInt("used") << sa::tab::endc <<
                 sa::tab::ralign << result->GetUInt("total") <<sa::tab::endc <<
                 result->GetString("description") << sa::tab::endr;
    }
    std::cout << table;
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

struct SkillStruct
{
    std::string uuid;
    std::string filename;
};

static bool UpdateSkill(DB::Database& db, const SkillStruct& skill)
{
    if (!Utils::FileExists(skill.filename))
    {
        std::cerr << "File " << skill.filename << " not found" << std::endl;
        return false;
    }
    LuaSkill lSkill;
    if (!lSkill.Execute(skill.filename))
        return false;

    if (sVerbose)
        std::cout << "Updating: " << skill.filename << std::endl;

    std::ostringstream query;
    query << "UPDATE `game_skills` SET ";
    query << " `activation` = " << lSkill.GetActivation() << ", ";
    query << " `recharge` = " << lSkill.GetRecharge() << ", ";
    query << " `const_energy` = " << lSkill.GetEnergy() << ", ";
    query << " `const_energy_regen` = " << lSkill.GetEnergyRegen() << ", ";
    query << " `const_adrenaline` = " << lSkill.GetAdrenaline() << ", ";
    query << " `const_overcast` = " << lSkill.GetOvercast() << ", ";
    query << " `const_hp` = " << lSkill.GetHp();

    query << " WHERE `uuid` = " << db.EscapeString(skill.uuid);
    if (sVerbose)
        std::cout << query.str() << std::endl;

    if (!sReadOnly)
    {
        if (!db.ExecuteQuery(query.str()))
            return false;
    }
    return true;
}

std::string gDataDir;

static bool UpdateSkills(DB::Database& db)
{
    const std::string exeFile = Utils::GetExeName();
    const std::string path = Utils::ExtractFileDir(exeFile);
    std::vector<SkillStruct> skills;

    std::string cfgFile = Utils::ConcatPath(path, "abserv.lua");
    IO::SimpleConfigManager cfg;
    if (!cfg.Load(cfgFile))
    {
        std::cerr << "Failed to load config file " << cfgFile << std::endl;
        return false;
    }
    gDataDir = cfg.GetGlobalString("data_dir", "");
    if (sVerbose)
        std::cout << "Data directory: " << gDataDir << std::endl;

    std::ostringstream query;
    query << "SELECT * FROM `game_skills`";
    for (std::shared_ptr<DB::DBResult> result = db.StoreQuery(query.str()); result; result = result->Next())
    {
        std::string file = result->GetString("script");
        if (file.empty())
            continue;

        skills.push_back({
            result->GetString("uuid"),
            Utils::ConcatPath(gDataDir, file)
        });
    }

    DB::DBTransaction transaction(&db);
    if (!transaction.Begin())
        return false;

    for (const auto& skill : skills)
    {
        if (!UpdateSkill(db, skill))
            return false;
    }

    if (!sReadOnly)
    {
        std::ostringstream queryVersion;
        queryVersion << "UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';";
        if (!db.ExecuteQuery(queryVersion.str()))
            return false;
    }

    return transaction.Commit();
}

/// What should we do.
/// At the moment we can only update the DB
enum class Action
{
    Update,
    Versions,
    AccountKeys,
    GenAccKey,
    UpdateSkills,
};

int main(int argc, char** argv)
{
    ShowLogo();

    const std::string exeFile = Utils::GetExeName();
    const std::string path = Utils::ExtractFileDir(exeFile);

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
    else if (sActval.compare("updateskills") == 0)
        action = Action::UpdateSkills;
    else
    {
        std::cerr << "Unknown action " << sActval << std::endl;
        ShowHelp(_cli);
        return EXIT_FAILURE;
    }
    sReadOnly = sa::arg_parser::get_value<bool>(parsedArgs, "readonly", false);
    sVerbose = sa::arg_parser::get_value<bool>(parsedArgs, "verbose", false);

    std::string cfgFile = Utils::ConcatPath(path, "config/db.lua");
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
        std::string schemasDir = sa::arg_parser::get_value<std::string>(parsedArgs, "schemadir", "");
        if (schemasDir.empty())
            schemasDir = cfg.GetGlobalString("db_schema_dir", "");

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
    else if (action == Action::UpdateSkills)
    {
        UpdateSkills(*db);
    }
    else
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
