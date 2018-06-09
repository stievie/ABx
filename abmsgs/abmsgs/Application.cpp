#include "stdafx.h"
#include "Application.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "SimpleConfigManager.h"
#include "StringUtils.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Utils.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    ioService_(),
    running_(false)
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
}

Application::~Application()
{
    Asynch::Scheduler::Instance.Stop();
    Asynch::Dispatcher::Instance.Stop();
}

bool Application::ParseCommandLine()
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
        else if (a.compare("-h") == 0 || a.compare("-help") == 0)
        {
            return false;
        }
    }
    return true;
}

void Application::ShowHelp()
{
    std::cout << "abmsgs [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

bool Application::LoadMain()
{
    if (configFile_.empty())
        configFile_ = path_ + "/abmsgs.lua";

    LOG_INFO << "Loading configuration...";
    if (!IO::SimpleConfigManager::Instance.Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
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
        IO::SimpleConfigManager::Instance.GetGlobal("message_ip", "0.0.0.0")
    ));
    uint16_t port = static_cast<uint16_t>(
        IO::SimpleConfigManager::Instance.GetGlobal("message_port", 2771)
    );

    asio::ip::tcp::endpoint endpoint(asio::ip::address(asio::ip::address_v4(ip)), port);
    server_ = std::make_unique<MessageServer>(ioService_, endpoint);

    PrintServerInfo();
    return true;
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << IO::SimpleConfigManager::Instance.GetGlobal("server_id", "") << std::endl;
    LOG_INFO << "  Location: " << IO::SimpleConfigManager::Instance.GetGlobal("location", "--") << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;

    LOG_INFO << "  Listening: ";
    LOG_INFO << IO::SimpleConfigManager::Instance.GetGlobal("message_ip", "0.0.0.0") << ":";
    LOG_INFO << IO::SimpleConfigManager::Instance.GetGlobal("message_port", 2771);
    LOG_INFO << std::endl;

    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
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

    return true;
}

void Application::Run()
{
    AB::Entities::Service serv;
    serv.uuid = IO::SimpleConfigManager::Instance.GetGlobal("server_id", "");
    dataClient_->Read(serv);
    serv.location = IO::SimpleConfigManager::Instance.GetGlobal("location", "--");
    serv.host = IO::SimpleConfigManager::Instance.GetGlobal("message_host", "");
    serv.port = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("message_port", 2771));
    serv.name = IO::SimpleConfigManager::Instance.GetGlobal("server_name", "abmsgs");
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeMessageServer;
    serv.startTime = Utils::AbTick();
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

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

    ioService_.stop();
}
