#include "stdafx.h"
#include "Application.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "SimpleConfigManager.h"
#include "StringUtils.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Utils.h"
#include "Subsystems.h"

Application::Application() :
    ServerApp::ServerApp(),
    ioService_()
{
    serverType_ = AB::Entities::ServiceTypeMessageServer;

    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(ioService_);
}

Application::~Application()
{
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
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

    auto config = GetSubsystem<IO::SimpleConfigManager>();
    LOG_INFO << "Loading configuration...";
    if (!config->Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }
    if (serverId_.empty() || uuids::uuid(serverId_).nil())
        serverId_ = config->GetGlobal("server_id", Utils::Uuid::EMPTY_UUID);
    if (serverName_.empty())
        serverName_ = config->GetGlobal("server_name", "abmsgs");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobal("location", "--");
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to data server...";
    auto dataClient = GetSubsystem<IO::DataClient>();
    const std::string& dataHost = config->GetGlobal("data_host", "");
    uint16_t dataPort = static_cast<uint16_t>(config->GetGlobal("data_port", 0));
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

    // Add Protocols
    uint32_t ip = static_cast<uint32_t>(Utils::ConvertStringToIP(
        config->GetGlobal("message_ip", "0.0.0.0")
    ));
    uint16_t port = static_cast<uint16_t>(
        config->GetGlobal("message_port", 2771)
    );

    asio::ip::tcp::endpoint endpoint(asio::ip::address(asio::ip::address_v4(ip)), port);
    server_ = std::make_unique<MessageServer>(ioService_, endpoint);

    PrintServerInfo();
    return true;
}

void Application::PrintServerInfo()
{
    auto config = GetSubsystem<IO::SimpleConfigManager>();
    auto dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;

    LOG_INFO << "  Listening: ";
    LOG_INFO << config->GetGlobal("message_ip", "0.0.0.0") << ":";
    LOG_INFO << config->GetGlobal("message_port", 2771);
    LOG_INFO << std::endl;

    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
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

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();

    if (!LoadMain())
        return false;

    return true;
}

void Application::Run()
{
    auto config = GetSubsystem<IO::SimpleConfigManager>();
    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient->Read(serv);
    serv.location = serverLocation_;
    serv.host = config->GetGlobal("message_host", "");
    serv.port = static_cast<uint16_t>(config->GetGlobal("message_port", 2771));
    serv.name = serverName_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = Utils::AbTick();
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

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

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (dataClient->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::AbTick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        dataClient->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient->Invalidate(sl);
    }
    LOG_ERROR << "Unable to read service" << std::endl;

    ioService_.stop();
}
