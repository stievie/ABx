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
#include "Bans.h"

Application* Application::Instance = nullptr;

Application::Application()
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
    serviceManager_ = std::make_unique<Net::ServiceManager>(ioService_);
}

Application::~Application()
{
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

bool Application::LoadMain()
{
    if (configFile_.empty())
        configFile_ = path_ + "/ablogin.lua";

    LOG_INFO << "Loading configuration...";
    if (!IO::SimpleConfigManager::Instance.Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    Net::ConnectionManager::maxPacketsPerSec = static_cast<uint32_t>(IO::SimpleConfigManager::Instance.GetGlobal("max_packets_per_second", 0));
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to data server...";
    const std::string& dataHost = IO::SimpleConfigManager::Instance.GetGlobal("data_host", "");
    uint16_t dataPort = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("data_port", 0));
    dataClient_->Connect(dataHost, dataPort);
    if (!dataClient_->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    // Add Protocols
    uint32_t ip = static_cast<uint32_t>(Utils::ConvertStringToIP(
        IO::SimpleConfigManager::Instance.GetGlobal("login_ip", "0.0.0.0")
    ));
    uint16_t port = static_cast<uint16_t>(
        IO::SimpleConfigManager::Instance.GetGlobal("login_port", 2748)
    );
    if (port != 0)
        serviceManager_->Add<Net::ProtocolLogin>(ip, port, [](uint32_t remoteIp) -> bool
    {
        return Auth::BanManager::Instance.AcceptConnection(remoteIp);
    });

    PrintServerInfo();
    return true;
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server Info:" << std::endl;
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

    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
}

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
        return false;

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

    if (!serviceManager_->IsRunning())
        LOG_ERROR << "No services running" << std::endl;

    return serviceManager_->IsRunning();
}

void Application::Run()
{
    AB::Entities::Service serv;
    serv.uuid = IO::SimpleConfigManager::Instance.GetGlobal("server_id", "");
    dataClient_->Read(serv);
    serv.host = IO::SimpleConfigManager::Instance.GetGlobal("login_host", "");
    serv.port = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("login_port", 2748));
    serv.name = "ablogin";
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeLoginServer;
    serv.startTime = Utils::AbTick();
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    LOG_INFO << "Server is running" << std::endl;
    serviceManager_->Run();
}

void Application::Stop()
{
    LOG_INFO << "Server is shutting down" << std::endl;
    AB::Entities::Service serv;
    serv.uuid = IO::SimpleConfigManager::Instance.GetGlobal("server_id", "");
    dataClient_->Read(serv);
    serv.status = AB::Entities::ServiceStatusOffline;
    serv.stopTime = Utils::AbTick();
    if (serv.startTime != 0)
        serv.runTime += (serv.stopTime - serv.startTime) / 1000;
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    serviceManager_->Stop();
}
