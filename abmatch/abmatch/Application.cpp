#include "stdafx.h"
#include "Application.h"
#include "Dispatcher.h"
#include "Scheduler.h"
#include "SimpleConfigManager.h"
#include "StringUtils.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "Subsystems.h"
#include "MatchQueues.h"

Application::Application() :
    ServerApp::ServerApp(),
    ioService_()
{
    serverType_ = AB::Entities::ServiceTypeMatchServer;

    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<MatchQueues>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(ioService_);
    Subsystems::Instance.CreateSubsystem<Net::MessageClient>(ioService_);
}

Application::~Application()
{
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
}

void Application::ShowHelp()
{
    std::cout << "abmatch [-<options> [<value>]]" << std::endl;
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
        configFile_ = path_ + "/" + "abmatch_svc.lua";
#else
        configFile_ = path_ + "/" + "abmatch.lua";
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
        serverName_ = config->GetGlobalString("server_name", "abmatch");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");
    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");

    LOG_INFO << "Connecting to data server...";
    auto* dataClient = GetSubsystem<IO::DataClient>();
    const std::string dataHost = config->GetGlobalString("data_host", "");
    uint16_t dataPort = static_cast<uint16_t>(config->GetGlobalInt("data_port", 0ll));
    dataClient->Connect(dataHost, dataPort);
    if (!dataClient->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Connecting to message server...";
    const std::string& msgHost = config->GetGlobalString("message_host", "");
    uint16_t msgPort = static_cast<uint16_t>(config->GetGlobalInt("message_port", 0ll));

    auto* msgClient = GetSubsystem<Net::MessageClient>();
    msgClient->Connect(msgHost, msgPort, std::bind(&Application::HandleMessage, this, std::placeholders::_1));
    if (msgClient->IsConnected())
        LOG_INFO << "[done]" << std::endl;
    else
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to message server" << std::endl;
    }

    PrintServerInfo();
    return true;
}

void Application::PrintServerInfo()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    auto* msgClient = GetSubsystem<Net::MessageClient>();
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;

    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
    LOG_INFO << "  Message Server: " << msgClient->GetHost() << ":" << msgClient->GetPort() << std::endl;
}

void Application::UpdateQueue()
{
    // Dispatcher thread
    int64_t tick = Utils::Tick();
    if (lastUpdate_ == 0)
        lastUpdate_ = tick - QUEUE_UPDATE_INTERVAL_MS;
    uint32_t delta = static_cast<uint32_t>(tick - lastUpdate_);
    lastUpdate_ = tick;
    GetSubsystem<MatchQueues>()->Update(delta);

    if (running_)
    {
        // Schedule next update
        const int64_t end = Utils::Tick();
        const uint32_t duration = static_cast<uint32_t>(end - lastUpdate_);
        const uint32_t sleepTime = QUEUE_UPDATE_INTERVAL_MS > duration ?
            QUEUE_UPDATE_INTERVAL_MS - duration : 0;
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(sleepTime, std::bind(&Application::UpdateQueue, this)));
    }
}

void Application::MainLoop()
{
    // Main thread
    while (running_)
    {
        ioService_.run();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
}

void Application::HandleQueueAdd(const Net::MessageMsg& msg)
{
    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
    {
        LOG_ERROR << "Error reading property stream from message" << std::endl;
        return;
    }
    std::string playerUuid;
    prop.ReadString(playerUuid);
    std::string mapUuid;
    prop.ReadString(mapUuid);
    GetSubsystem<MatchQueues>()->Add(mapUuid, playerUuid);
}

void Application::HandleQueueRemove(const Net::MessageMsg& msg)
{
    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
    {
        LOG_ERROR << "Error reading property stream from message" << std::endl;
        return;
    }
    std::string playerUuid;
    prop.ReadString(playerUuid);
    GetSubsystem<MatchQueues>()->Remove(playerUuid);
}

void Application::HandleMessage(const Net::MessageMsg& msg)
{
    // Main thread
    switch (msg.type_)
    {
    case Net::MessageType::QueueAdd:
        HandleQueueAdd(msg);
        break;
    case Net::MessageType::QueueRemove:
        HandleQueueRemove(msg);
        break;
    default:
        break;
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

    return true;
}

void Application::Run()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient->Read(serv);
    serv.location = serverLocation_;
    serv.host = "";
    serv.port = 0;
    serv.ip = "";
    serv.name = serverName_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

    GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(QUEUE_UPDATE_INTERVAL_MS, std::bind(&Application::UpdateQueue, this)));
    // If we want to receive messages, we need to send our ServerID to the message server.
    SendServerJoined(GetSubsystem<Net::MessageClient>(), serv);

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;

    MainLoop();
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

        SendServerLeft(GetSubsystem<Net::MessageClient>(), serv);

        dataClient->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;

    ioService_.stop();
}
