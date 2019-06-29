#include "stdafx.h"
#include "Application.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "SimpleConfigManager.h"
#include "Connection.h"
#include "StringUtils.h"
#include "ProtocolLogin.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Utils.h"
#include "BanManager.h"
#include "Subsystems.h"
#include "Random.h"
#include <AB/DHKeys.hpp>
#include "FileUtils.h"
#include "DataClient.h"

Application::Application() :
    ServerApp::ServerApp(),
    ioService_()
{
    serverType_ = AB::Entities::ServiceTypeLoginServer;

    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<Net::ConnectionManager>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(ioService_);
    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    Subsystems::Instance.CreateSubsystem<Crypto::Random>();
    Subsystems::Instance.CreateSubsystem<Crypto::DHKeys>();

    serviceManager_ = std::make_unique<Net::ServiceManager>(ioService_);
}

Application::~Application()
{
    serviceManager_->Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
    GetSubsystem<Net::ConnectionManager>()->CloseAll();
}

void Application::ShowHelp()
{
    std::cout << "ablogin [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

bool Application::LoadMain()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = path_ + "/" + "ablogin_svc.lua";
#else
        configFile_ = path_ + "/" + "ablogin.lua";
#endif
    }

    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    LOG_INFO << "Loading configuration...";
    if (!config->Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    if (serverId_.empty() || uuids::uuid(serverId_).nil())
        serverId_ = config->GetGlobalString("server_id", Utils::Uuid::EMPTY_UUID);
    if (serverName_.empty())
        serverName_ = config->GetGlobalString("server_name", "ablogin");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");
    if (serverHost_.empty())
        serverHost_ = config->GetGlobalString("login_host", "");
    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");

    Net::ConnectionManager::maxPacketsPerSec = static_cast<uint32_t>(config->GetGlobalInt("max_packets_per_second", 0ll));
    Auth::BanManager::LoginTries = static_cast<uint32_t>(config->GetGlobalInt("login_tries", 5ll));
    Auth::BanManager::LoginRetryTimeout = static_cast<uint32_t>(config->GetGlobalInt("login_retrytimeout", 5000ll));

    LOG_INFO << "Initializing RNG...";
    GetSubsystem<Crypto::Random>()->Initialize();
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Loading encryption keys...";
    auto* keys = GetSubsystem<Crypto::DHKeys>();
    if (!keys)
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to get encryption keys" << std::endl;
        return false;
    }
    if (!keys->LoadKeys(GetKeysFile()))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to load encryption keys from " << GetKeysFile() << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to data server...";
    auto* dataClient = GetSubsystem<IO::DataClient>();
    const std::string& dataHost = config->GetGlobalString("data_host", "");
    uint16_t dataPort = static_cast<uint16_t>(config->GetGlobalInt("data_port", 0ll));
    dataClient->Connect(dataHost, dataPort);
    if (!dataClient->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;
    if (serverName_.empty() || serverName_.compare("generic") == 0)
    {
        serverName_ = GetFreeName(dataClient);
    }

    if (serverIp_.empty())
        serverIp_ = config->GetGlobalString("login_ip", "0.0.0.0");
    if (serverPort_ == std::numeric_limits<uint16_t>::max())
        serverPort_ = static_cast<uint16_t>(config->GetGlobalInt("login_port", 2748ll));
    else if (serverPort_ == 0)
        serverPort_ = Net::ServiceManager::GetFreePort();

    // Add Protocols
    uint32_t ip = static_cast<uint32_t>(Utils::ConvertStringToIP(serverIp_));
    if (serverPort_ != 0)
    {
        if (!serviceManager_->Add<Net::ProtocolLogin>(ip, serverPort_, [](uint32_t remoteIp) -> bool
        {
            return GetSubsystem<Auth::BanManager>()->AcceptConnection(remoteIp);
        }))
            return false;
    }
    else
    {
        LOG_ERROR << "Port can not be 0" << std::endl;
        return false;
    }

    PrintServerInfo();
    return true;
}

void Application::PrintServerInfo()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Protocol version: " << AB::PROTOCOL_VERSION << std::endl;

    std::list<std::pair<uint32_t, uint16_t>> ports = serviceManager_->GetPorts();
    LOG_INFO << "  Listening: ";
    while (ports.size())
    {
        LOG_INFO << Utils::ConvertIPToString(ports.front().first) << ":" << ports.front().second << " ";
        ports.pop_front();
    }
    LOG_INFO << std::endl;

    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
}

void Application::HeardBeatTask()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    if (dataClient->IsConnected())
    {
        AB::Entities::Service serv;
        serv.uuid = serverId_;
        if (dataClient->Read(serv))
        {
            serv.heardbeat = Utils::Tick();
            if (!dataClient->Update(serv))
                LOG_ERROR << "Error updating service " << serverId_ << std::endl;
        }
        else
            LOG_ERROR << "Error reading service " << serverId_ << std::endl;
    }
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(AB::Entities::HEARDBEAT_INTERVAL, std::bind(&Application::HeardBeatTask, this))
        );
    }
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

    if (!LoadMain())
        return false;

    if (!logDir_.empty())
    {
        // From the command line
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();

    if (!serviceManager_->IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_->IsRunning();
}

void Application::Run()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient->Read(serv);
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
    serv.heardbeat = Utils::Tick();
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(AB::Entities::HEARDBEAT_INTERVAL, std::bind(&Application::HeardBeatTask, this))
    );

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    serviceManager_->Run();
    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (dataClient->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::Tick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        dataClient->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;

    ioService_.stop();
}

std::string Application::GetKeysFile() const
{
    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    const std::string keys = config->GetGlobalString("server_keys", "");
    if (!keys.empty())
        return keys;
    return Utils::AddSlash(path_) + "abserver.dh";
}
