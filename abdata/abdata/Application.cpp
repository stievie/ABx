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

Application::Application() :
    ServerApp::ServerApp(),
    port_(0),
    listenIp_(0),
    maxSize_(0),
    readonly_(false),
    running_(false),
    ioService_(),
    server_(nullptr),
    flushInterval_(FLUSH_CACHE_MS),
    cleanInterval_(CLEAN_CACHE_MS)
{
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
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
    for (size_t i = 0; i < arguments_.size(); i++)
    {
        const std::string& arg = arguments_[i];
        if (arg.compare("-port") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                port_ = static_cast<uint16_t>(std::atoi(arguments_[i].c_str()));
            }
            else
                LOG_WARNING << "Missing argument for -port" << std::endl;
        }
        if (arg.compare("-ip") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                listenIp_ = Utils::ConvertStringToIP(arguments_[i]);
            }
            else
                LOG_WARNING << "Missing argument for -port" << std::endl;
        }
        else if (arg.compare("-maxsize") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                maxSize_ = static_cast<size_t>(std::atoi(arguments_[i].c_str()));
            }
            else
                LOG_WARNING << "Missing argument for -maxsize" << std::endl;
        }
        else if (arg.compare("-readonly") == 0)
        {
            readonly_ = true;
            i++;
        }
        else if (arg.compare("-log") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                IO::Logger::logDir_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
        else if (arg.compare("-conf") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                configFile_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
        else if (arg.compare("-dbdriver") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                DB::Database::driver_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -dbdriver" << std::endl;
        }
        else if (arg.compare("-dbhost") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                DB::Database::dbHost_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -dbhost" << std::endl;
        }
        else if (arg.compare("-dbname") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                DB::Database::dbName_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -dbname" << std::endl;
        }
        else if (arg.compare("-dbuser") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                DB::Database::dbUser_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -dbuser" << std::endl;
        }
        else if (arg.compare("-dbpass") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                DB::Database::dbPass_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -dbpass" << std::endl;
        }
        else if (arg.compare("-dbport") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                i++;
                DB::Database::dbPort_ = static_cast<uint16_t>(std::atoi(arguments_[i].c_str()));
            }
            else
                LOG_WARNING << "Missing argument for -dbport" << std::endl;
        }
        else if (arg.compare("-h") == 0 || arg.compare("-help") == 0)
        {
            return false;
        }
    }
    return true;
}

bool Application::LoadConfig()
{
    if (configFile_.empty())
        configFile_ = path_ + "/" + "abdata.lua";

    auto config = GetSubsystem<IO::SimpleConfigManager>();
    if (Utils::FileExists(configFile_))
    {
        if (!config->Load(configFile_))
        {
            LOG_ERROR << "Error loading config file" << std::endl;
            return false;
        }
    }

    if (port_ == 0)
        port_ = static_cast<uint16_t>(config->GetGlobal("data_port", 0));
    if (listenIp_ == 0)
        listenIp_ = Utils::ConvertStringToIP(config->GetGlobal("data_ip", ""));
    if (maxSize_ == 0)
        maxSize_ = config->GetGlobal("max_size", 0);
    if (!readonly_)
        readonly_ = config->GetGlobalBool("read_only", false);
    if (IO::Logger::logDir_.empty())
        IO::Logger::logDir_ = config->GetGlobal("log_dir", "");
    if (DB::Database::driver_.empty())
        DB::Database::driver_ = config->GetGlobal("db_driver", "");
    if (DB::Database::dbHost_.empty())
        DB::Database::dbHost_ = config->GetGlobal("db_host", "");
    if (DB::Database::dbName_.empty())
        DB::Database::dbName_ = config->GetGlobal("db_name", "");
    if (DB::Database::dbUser_.empty())
        DB::Database::dbUser_ = config->GetGlobal("db_user", "");
    if (DB::Database::dbPass_.empty())
        DB::Database::dbPass_ = config->GetGlobal("db_pass", "");
    if (DB::Database::dbPort_ == 0)
        DB::Database::dbPort_ = static_cast<uint16_t>(config->GetGlobal("db_port", 0));

    flushInterval_ = static_cast<uint32_t>(config->GetGlobal("flush_interval", (int64_t)flushInterval_));
    cleanInterval_ = static_cast<uint32_t>(config->GetGlobal("clean_interval", (int64_t)cleanInterval_));

    if (port_ == 0)
    {
        LOG_ERROR << "Port is 0" << std::endl;
    }
    if (maxSize_ == 0)
    {
        LOG_ERROR << "Cache size is 0" << std::endl;
    }
    return (port_ != 0) && (maxSize_ != 0);
}

void Application::PrintServerInfo()
{
    auto config = GetSubsystem<IO::SimpleConfigManager>();
    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << config->GetGlobal("server_id", "") << std::endl;
    LOG_INFO << "  Location: " << config->GetGlobal("location", "--") << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << Utils::ConvertIPToString(listenIp_) << ":" << port_ << std::endl;
    LOG_INFO << "  Cache size: " << Utils::ConvertSize(maxSize_) << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Readonly mode: " << (readonly_ ? "TRUE" : "false") << std::endl;
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

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
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
    DB::Database* db = DB::Database::Instance();
    if (db == nullptr || !db->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Database connection failed" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    PrintServerInfo();

    return true;
}

void Application::Run()
{
    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();

    server_ = std::make_unique<Server>(ioService_, listenIp_, port_, maxSize_, readonly_);
    StorageProvider* provider = server_->GetStorageProvider();
    provider->flushInterval_ = flushInterval_;
    provider->cleanInterval_ = cleanInterval_;

    auto config = GetSubsystem<IO::SimpleConfigManager>();
    AB::Entities::Service serv;
    serv.uuid = config->GetGlobal("server_id", "");
    provider->EntityRead(serv);
    serv.location = config->GetGlobal("location", "--");
    serv.host = config->GetGlobal("data_host", "");
    serv.port = static_cast<uint16_t>(config->GetGlobal("data_port", 2770));
    serv.name = config->GetGlobal("server_name", "abdata");
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeDataServer;
    serv.startTime = Utils::AbTick();
    provider->EntityUpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    provider->EntityInvalidate(sl);

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown...";

    StorageProvider* provider = server_->GetStorageProvider();
    AB::Entities::Service serv;
    serv.uuid = GetSubsystem<IO::SimpleConfigManager>()->GetGlobal("server_id", "");
    if (provider->EntityRead(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::AbTick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        provider->EntityUpdate(serv);

        AB::Entities::ServiceList sl;
        provider->EntityInvalidate(sl);
    }
    else
        LOG_ERROR << "Error reading service" << std::endl;

    server_->Shutdown();
}
