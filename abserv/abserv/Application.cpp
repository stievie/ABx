#include "stdafx.h"
#include "Application.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ProtocolAdmin.h"
#include "ProtocolStatus.h"
#include "ConfigManager.h"
#include "Task.h"
#include "Logger.h"
#include "StringUtils.h"
#include "GameManager.h"
#include <functional>
#include "Random.h"
#include "Connection.h"
#include "SkillManager.h"
#include "Skill.h"
#include "EffectManager.h"
#include "DataProvider.h"
#include "Maintenance.h"
#include "Utils.h"
#include <AB/ProtocolCodes.h>
#include <base64.h>
#include "Profiler.h"
#include "PlayerManager.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Connection.h"
#include "Bans.h"

#include "DebugNew.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    ioService_()
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
    serviceManager_ = std::make_unique<Net::ServiceManager>(ioService_);
}

Application::~Application()
{
    Game::GameManager::Instance.Stop();
    Asynch::Scheduler::Instance.Stop();
    Asynch::Dispatcher::Instance.Stop();
}

void Application::ParseCommandLine()
{
    for (int i = 0; i != arguments_.size(); i++)
    {
        const std::string& a = arguments_[i];
        if (a.compare("-conf") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                configFile_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -conf" << std::endl;
        }
        else if (a.compare("-log") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                logDir_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -log" << std::endl;
        }
    }
}

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
        return false;
    using namespace std::chrono_literals;
    ParseCommandLine();
    if (!logDir_.empty())
    {
        // From the command line
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    Asynch::Dispatcher::Instance.Start();
    Asynch::Scheduler::Instance.Start();

    if (!LoadMain())
        return false;

    std::this_thread::sleep_for(100ms);

    if (!serviceManager_->IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_->IsRunning();
}

bool Application::LoadMain()
{
    int64_t startLoading = Utils::AbTick();

    LOG_INFO << "Loading..." << std::endl;

    if (configFile_.empty())
        configFile_ = path_ + "/" + CONFIG_FILE;
    LOG_INFO << "Loading configuration: " << configFile_ << "...";
    if (!ConfigManager::Instance.Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    Net::ConnectionManager::maxPacketsPerSec = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::MaxPacketsPerSecond].GetInt64());
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Initializing RNG...";
    Utils::Random::Instance.Initialize();
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to data server...";
    const std::string& dataHost = ConfigManager::Instance[ConfigManager::Key::DataServerHost].GetString();
    uint16_t dataPort = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::DataServerPort].GetInt());
    dataClient_->Connect(dataHost, dataPort);
    if (!dataClient_->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    // Add Protocols
    uint32_t ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::AdminIP].GetInt());
    uint16_t port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::AdminPort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolAdmin>(ip, port, [](uint32_t remoteIp) -> bool
    {
        return Auth::BanManager::Instance.AcceptConnection(remoteIp);
    });
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::StatusIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::StatusPort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolStatus>(ip, port, [](uint32_t remoteIp) -> bool
    {
        return Auth::BanManager::Instance.AcceptConnection(remoteIp);
    });
    ip = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::Key::GameIP].GetInt());
    port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::GamePort].GetInt());
    if (port != 0)
        serviceManager_->Add<Net::ProtocolGame>(ip, port, [](uint32_t remoteIp) -> bool
    {
        return Auth::BanManager::Instance.AcceptConnection(remoteIp);
    });

    int64_t loadingTime = (Utils::AbTick() - startLoading);

    PrintServerInfo();

    LOG_INFO << "Loading done in ";
    if (loadingTime < 1000)
        LOG_INFO << loadingTime << " ms";
    else
        LOG_INFO << (loadingTime / 1000) << " s";
    LOG_INFO << std::endl;

    Maintenance::Instance.Run();
    Game::GameManager::Instance.Start();

    return true;
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server name: " << ConfigManager::Instance[ConfigManager::Key::ServerName].GetString() << std::endl;
    LOG_INFO << "  Location: " << ConfigManager::Instance[ConfigManager::Key::Location].GetString() << std::endl;
    LOG_INFO << "  Protocol version: " << AB::PROTOCOL_VERSION << std::endl;

    std::list<std::pair<uint32_t, uint16_t>> ports = serviceManager_->GetPorts();
    LOG_INFO << "  Listening: ";
    while (ports.size())
    {
        LOG_INFO << Utils::ConvertIPToString(ports.front().first) << ":" << ports.front().second << " ";
        ports.pop_front();
    }
    LOG_INFO << std::endl;

    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
}

void Application::Run()
{
    AB::Entities::Service serv;
    serv.uuid = ConfigManager::Instance[ConfigManager::Key::ServerID].GetString();
    dataClient_->Read(serv);
    serv.host = ConfigManager::Instance[ConfigManager::Key::GameHost].GetString();
    serv.port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::GamePort].GetInt());
    serv.statusPort = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::Key::StatusPort].GetInt());
    serv.name = "abserv";
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeGameServer;
    serv.startTime = Utils::AbTick();
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    LOG_INFO << "Server is running" << std::endl;
    // If we use a log file close current and reopen as file logger
    if (logDir_.empty())
        logDir_ = ConfigManager::Instance[ConfigManager::Key::LogDir].GetString();
    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        LOG_INFO << "Log directory: " << logDir_ << std::endl;
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }
    serviceManager_->Run();
}

void Application::Stop()
{
    LOG_INFO << "Server is shutting down" << std::endl;
    AB::Entities::Service serv;
    serv.uuid = ConfigManager::Instance[ConfigManager::Key::ServerID].GetString();
    dataClient_->Read(serv);
    serv.status = AB::Entities::ServiceStatusOffline;
    serv.stopTime = Utils::AbTick();
    if (serv.startTime != 0)
        serv.runTime += (serv.stopTime - serv.startTime) / 1000;
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    Game::PlayerManager::Instance.KickAllPlayers();
    // Before serviceManager_.Stop()
    Maintenance::Instance.Stop();

    serviceManager_->Stop();
}
