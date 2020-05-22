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


#include "Application.h"
#include "Version.h"
#include "Server.h"
#include <abscommon/Logo.h>
#include <abdb/Database.h>
#include <abscommon/StringUtils.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Scheduler.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include <abscommon/Utils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/ThreadPool.h>
#include <abscommon/UuidUtils.h>
#include <filesystem>
#include <sa/StringTempl.h>

Application::Application() :
    ServerApp::ServerApp(),
    listenIp_(0),
    maxSize_(0),
    readonly_(false),
    ioService_(),
    flushInterval_(FLUSH_CACHE_MS),
    cleanInterval_(CLEAN_CACHE_MS)
{
    serverType_ = AB::Entities::ServiceTypeDataServer;
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<Asynch::ThreadPool>(1);
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();

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

    cli_.push_back({ "maxsize", { "-maxsize", "--maxsize-cache" }, "Maximum cache size in bytes",
        false, true, sa::arg_parser::option_type::integer });
    cli_.push_back({ "readonly", { "-readonly", "--readonly-mode" }, "Read only mode. Do not write to DB",
        false, false, sa::arg_parser::option_type::none });
    cli_.push_back({ "dbdriver", { "-dbdriver", "--database-driver" }, "Database driver, possible value(s):" + dbDrivers.str(),
        false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "dbhost", { "-dbhost", "--database-host" }, "Host name of database server",
        false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "dbport", { "-dbport", "--database-port" }, "Port to connect to",
        false, true, sa::arg_parser::option_type::integer });
    cli_.push_back({ "dbname", { "-dbname", "--database-name" }, "Name of database",
        false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "dbuser", { "-dbuser", "--database-user" }, "User name for database",
        false, true, sa::arg_parser::option_type::string });
    cli_.push_back({ "dbpass", { "-dbpass", "--database-password" }, "Password for database",
        false, true, sa::arg_parser::option_type::string });
}

Application::~Application()
{
    if (running_)
        Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
}

void Application::ShowLogo()
{
    std::cout << "This is " << SERVER_PRODUCT_NAME << std::endl;
    std::cout << "Version " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-" << SERVER_YEAR << std::endl;
    std::cout << std::endl;

    std::cout << AB_CONSOLE_LOGO << std::endl;

    std::cout << std::endl;
}

bool Application::ParseCommandLine()
{
    if (!ServerApp::ParseCommandLine())
        return false;

    if (!serverIp_.empty())
        listenIp_ = Utils::ConvertStringToIP(serverIp_);
    if (!logDir_.empty())
        IO::Logger::logDir_ = logDir_;

    maxSize_ = sa::arg_parser::get_value<size_t>(parsedArgs_, "maxsize", maxSize_);
    readonly_ = sa::arg_parser::get_value<bool>(parsedArgs_, "readonly", false);
    DB::Database::driver_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "dbdriver", DB::Database::driver_);
    DB::Database::dbHost_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "dbhost", DB::Database::dbHost_);
    DB::Database::dbName_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "dbname", DB::Database::dbName_);
    DB::Database::dbUser_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "dbuser", DB::Database::dbUser_);
    DB::Database::dbPass_ = sa::arg_parser::get_value<std::string>(parsedArgs_, "dbpass", DB::Database::dbPass_);
    DB::Database::dbPort_ = sa::arg_parser::get_value<uint16_t>(parsedArgs_, "dbport", DB::Database::dbPort_);

    return true;
}

void Application::ShowVersion()
{
    std::cout << SERVER_PRODUCT_NAME << " " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
}

bool Application::LoadConfig()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = Utils::ConcatPath(path_, "abdata_svc.lua");
#else
        configFile_ = Utils::ConcatPath(path_, "abdata.lua");
#endif
    }

    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    if (Utils::FileExists(configFile_))
    {
        if (!config->Load(configFile_))
        {
            LOG_ERROR << "Error loading config file" << std::endl;
            return false;
        }
    }

    if (Utils::Uuid::IsEmpty(serverId_))
        serverId_ = config->GetGlobalString("server_id", Utils::Uuid::EMPTY_UUID);
    if (serverName_.empty())
        serverName_ = config->GetGlobalString("server_name", "abdata");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");

    if (serverPort_ == std::numeric_limits<uint16_t>::max())
        serverPort_ = static_cast<uint16_t>(config->GetGlobalInt("data_port", 0ll));
    if (listenIp_ == 0)
        listenIp_ = Utils::ConvertStringToIP(config->GetGlobalString("data_ip", ""));
    if (serverHost_.empty())
        serverHost_ = config->GetGlobalString("data_host", "");
    if (maxSize_ == 0)
        maxSize_ = static_cast<size_t>(config->GetGlobalInt("max_size", 0ll));
    if (!readonly_)
        readonly_ = config->GetGlobalBool("read_only", false);

    std::string ips = config->GetGlobalString("allowed_ips", "");
    whiteList_.AddList((ips));

    if (IO::Logger::logDir_.empty())
        IO::Logger::logDir_ = config->GetGlobalString("log_dir", "");
    if (DB::Database::driver_.empty())
        DB::Database::driver_ = config->GetGlobalString("db_driver", "");
    if (DB::Database::dbHost_.empty())
        DB::Database::dbHost_ = config->GetGlobalString("db_host", "");
    if (DB::Database::dbName_.empty())
        DB::Database::dbName_ = config->GetGlobalString("db_name", "");
    if (DB::Database::dbUser_.empty())
        DB::Database::dbUser_ = config->GetGlobalString("db_user", "");
    if (DB::Database::dbPass_.empty())
        DB::Database::dbPass_ = config->GetGlobalString("db_pass", "");
    if (DB::Database::dbPort_ == 0)
        DB::Database::dbPort_ = static_cast<uint16_t>(config->GetGlobalInt("db_port", 0ll));

    flushInterval_ = static_cast<uint32_t>(config->GetGlobalInt("flush_interval", flushInterval_));
    cleanInterval_ = static_cast<uint32_t>(config->GetGlobalInt("clean_interval", cleanInterval_));

    if (serverPort_ == 0)
    {
        LOG_ERROR << "Port is 0" << std::endl;
    }
    if (maxSize_ == 0)
    {
        LOG_ERROR << "Cache size is 0" << std::endl;
    }
    return (serverPort_ != 0) && (maxSize_ != 0);
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Machine: " << machine_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << Utils::ConvertIPToString(listenIp_) << ":" << serverPort_ << std::endl;
    LOG_INFO << "  Background threads: " << GetSubsystem<Asynch::ThreadPool>()->GetNumThreads() << std::endl;
    LOG_INFO << "  Cache size: " << Utils::ConvertSize(maxSize_) << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Readonly mode: " << (readonly_ ? "TRUE" : "false") << std::endl;
    LOG_INFO << "  Allowed IPs: ";
    if (whiteList_.IsEmpty())
        LOG_INFO << "(all)";
    else
        LOG_INFO << whiteList_.ToString();
    LOG_INFO << std::endl;
    LOG_INFO << "Database drivers:";
#ifdef USE_SQLITE
    LOG_INFO << " SQLite";
#endif
#ifdef USE_MYSQL
    LOG_INFO << " MySQL";
#endif
#ifdef USE_PGSQL
    LOG_INFO << " PostgreSQL";
#endif
#ifdef USE_ODBC
    LOG_INFO << " ODBC";
#endif
    LOG_INFO << std::endl;
    LOG_INFO << "Database config:" << std::endl;
    LOG_INFO << "  Driver: " << DB::Database::driver_ << std::endl;
    LOG_INFO << "  Host: " << DB::Database::dbHost_ << std::endl;
    LOG_INFO << "  Name: " << DB::Database::dbName_ << std::endl;
    LOG_INFO << "  Port: " << DB::Database::dbPort_ << std::endl;
    LOG_INFO << "  User: " << DB::Database::dbUser_ << std::endl;
    LOG_INFO << "  Password: " << (DB::Database::dbPass_.empty() ? "(empty)" : "***********") << std::endl;
}

int Application::GetDatabaseVersion()
{
    auto* db = GetSubsystem<DB::Database>();
    std::ostringstream query;
#ifdef USE_PGSQL
    if (DB::Database::driver_ == "pgsql")
        query << "SET search_path TO schema, public; ";
#endif
    query << "SELECT `value` FROM `versions` WHERE ";
    query << "`name` = " << db->EscapeString("schema");

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
    {
        // No such record means not even the first file (file 0) was imported
        return -1;
    }

    return static_cast<int>(result->GetUInt("value"));
}

bool Application::CheckDatabaseVersion()
{
    namespace fs = std::filesystem;

    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    if (!config->GetGlobalBool("db_version_check", true))
    {
        return true;
    }

    std::string schemasDir = config->GetGlobalString("db_schema_dir", "");
    if (schemasDir.empty())
    {
        LOG_WARNING << "Schemas directory is empty, skillping database check" << std::endl;
        return true;
    }

    LOG_INFO << "Checking Database version...";

    if (!fs::is_directory(schemasDir))
    {
        LOG_INFO << "[WARN]" << std::endl;
        LOG_WARNING << "SQL directory " << schemasDir << " does not exist" << std::endl;
        return true;
    }
    int expectedVer = -1;
    for (const auto& entry : fs::directory_iterator(schemasDir))
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
        if (expectedVer < static_cast<int>(version))
            expectedVer = static_cast<int>(version);
    }

    int currVer = GetDatabaseVersion();
    if (currVer == -1)
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Database is not initialized. Run `dbtool -a update`" << std::endl;
        return false;
    }
    if (expectedVer == -1)
    {
        LOG_INFO << "[WARN]" << std::endl;
        LOG_WARNING << "No *.sql files found in " << schemasDir << std::endl;
        // Do not fail when there is no sql directory
        return true;
    }
    if (currVer != expectedVer)
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Database has wrong version. Expected " << expectedVer << " but got " << currVer <<
            ". Please run `dbtool -a update`" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    return true;
}

bool Application::Initialize(const std::vector<std::string>& args)
{
    if (!ServerApp::Initialize(args))
        return false;
    if (!ParseCommandLine())
        return false;

    if (!sa::arg_parser::get_value<bool>(parsedArgs_, "nologo", false))
        ShowLogo();

    LOG_INFO << "Reading config file...";
    if (!LoadConfig())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    if (!IO::Logger::logDir_.empty())
        IO::Logger::Close();

    LOG_INFO << "Connecting to database...";
    DB::Database* db = DB::Database::CreateInstance(DB::Database::driver_,
        DB::Database::dbHost_, DB::Database::dbPort_,
        DB::Database::dbUser_, DB::Database::dbPass_,
        DB::Database::dbName_
    );
    if (!db || !db->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Database connection failed" << std::endl;
        return false;
    }
    Subsystems::Instance.RegisterSubsystem<DB::Database>(db);
    LOG_INFO << "[done]" << std::endl;
    if (!CheckDatabaseVersion())
        return false;

    PrintServerInfo();

    return true;
}

void Application::Run()
{
    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();
    GetSubsystem<Asynch::ThreadPool>()->Start();

    server_ = ea::make_unique<Server>(ioService_, listenIp_, serverPort_, maxSize_, readonly_, whiteList_);
    auto& provider = server_->GetStorageProvider();
    provider.flushInterval_ = flushInterval_;
    provider.cleanInterval_ = cleanInterval_;

    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    provider.EntityRead(serv);
    serv.location = serverLocation_;
    serv.host = serverHost_;
    serv.port = serverPort_;
    serv.ip = serverIp_;
    serv.name = serverName_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = sa::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
    serv.version = AB_SERVER_VERSION;
    provider.EntityUpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    provider.EntityInvalidate(sl);

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;

    auto& provider = server_->GetStorageProvider();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (provider.EntityRead(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::Tick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        provider.EntityUpdate(serv);

        AB::Entities::ServiceList sl;
        provider.EntityInvalidate(sl);
    }
    else
        LOG_ERROR << "Error reading service" << std::endl;

    server_->Shutdown();
}
