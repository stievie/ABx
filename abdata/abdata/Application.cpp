#include "stdafx.h"
#include "Application.h"
#include "Database.h"
#include "StringUtils.h"
#include "SimpleConfigManager.h"
#include "FileUtils.h"
#include "Scheduler.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Utils.h"
#include "Subsystems.h"
#include "ThreadPool.h"

Application::Application() :
    ServerApp::ServerApp(),
    listenIp_(0),
    maxSize_(0),
    readonly_(false),
    ioService_(),
    server_(nullptr),
    flushInterval_(FLUSH_CACHE_MS),
    cleanInterval_(CLEAN_CACHE_MS)
{
    serverType_ = AB::Entities::ServiceTypeDataServer;
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<Asynch::ThreadPool>(1);
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
}

Application::~Application()
{
    if (running_)
        Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
}

bool Application::ParseCommandLine()
{
    if (!ServerApp::ParseCommandLine())
        return false;

    if (!serverIp_.empty())
        listenIp_ = Utils::ConvertStringToIP(serverIp_);
    std::string value;
    if (GetCommandLineValue("-maxsize", value))
    {
        maxSize_ = static_cast<size_t>(std::atoi(value.c_str()));
    }
    if (GetCommandLineValue("-readonly"))
        readonly_ = true;

    if (!logDir_.empty())
        IO::Logger::logDir_ = logDir_;

    GetCommandLineValue("-dbdriver", DB::Database::driver_);
    GetCommandLineValue("-dbhost", DB::Database::dbHost_);
    GetCommandLineValue("-dbname", DB::Database::dbName_);
    GetCommandLineValue("-dbuser", DB::Database::dbUser_);
    GetCommandLineValue("-dbpass", DB::Database::dbPass_);
    value.clear();
    if (GetCommandLineValue("-dbport", value))
    {
        if (!value.empty())
            DB::Database::dbPort_ = static_cast<uint16_t>(std::atoi(value.c_str()));
    }

    return true;
}

bool Application::LoadConfig()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = path_ + "/" + "abdata_svc.lua";
#else
        configFile_ = path_ + "/" + "abdata.lua";
#endif
    }

    auto config = GetSubsystem<IO::SimpleConfigManager>();
    if (Utils::FileExists(configFile_))
    {
        if (!config->Load(configFile_))
        {
            LOG_ERROR << "Error loading config file" << std::endl;
            return false;
        }
    }

    if (serverId_.empty() || uuids::uuid(serverId_).nil())
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

void Application::ShowHelp()
{
    std::cout << "abdata [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  port <value>: The port it listens" << std::endl;
    std::cout << "  ip <value>: The ip it listens" << std::endl;
    std::cout << "  maxsize <value>: Maximum cache size in byte" << std::endl;
    std::cout << "  readonly: Readonly mode" << std::endl;
    std::cout << "  log <value>: Log directory" << std::endl;
    std::cout << "  conf <value>: Configuration file" << std::endl;
    std::cout << "  dbdriver <value>: Database driver" << std::endl;
    std::cout << "  dbhost <value>: Database host" << std::endl;
    std::cout << "  dbport <value>: Database port" << std::endl;
    std::cout << "  dbname <value>: Database name" << std::endl;
    std::cout << "  dbuser <value>: Database user" << std::endl;
    std::cout << "  dbpass <value>: Database password" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

bool Application::Initialize(const std::vector<std::string>& args)
{
    if (!ServerApp::Initialize(args))
        return false;
    if (!ParseCommandLine())
    {
        ShowHelp();
        return false;
    }
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

    PrintServerInfo();

    return true;
}

void Application::Run()
{
    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();
    GetSubsystem<Asynch::ThreadPool>()->Start();

    server_ = std::make_unique<Server>(ioService_, listenIp_, serverPort_, maxSize_, readonly_, whiteList_);
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
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
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
    serv.uuid = GetSubsystem<IO::SimpleConfigManager>()->GetGlobalString("server_id", "");
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
