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
#include "MatchQueues.h"
#include "Version.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/Logo.h>
#include <abscommon/Scheduler.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>

Application::Application() :
    ServerApp(),
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

bool Application::LoadMain()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = Utils::ConcatPath(path_, "abmatch_svc.lua");
#else
        configFile_ = Utils::ConcatPath(path_, "abmatch.lua");
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

    if (Utils::Uuid::IsEmpty(serverId_))
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
    LOG_INFO << "  Machine: " << machine_ << std::endl;
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
    sa::PropReadStream prop;
    if (!msg.GetPropStream(prop))
    {
        LOG_ERROR << "Error reading property stream from message" << std::endl;
        return;
    }
    std::string playerUuid;
    prop.ReadString(playerUuid);
    std::string mapUuid;
    prop.ReadString(mapUuid);
#ifdef DEBUG_MATCH
    LOG_DEBUG << "Player " << playerUuid << " queuing for " << mapUuid << std::endl;
#endif
    GetSubsystem<MatchQueues>()->Add(mapUuid, playerUuid);
}

void Application::HandleQueueRemove(const Net::MessageMsg& msg)
{
    sa::PropReadStream prop;
    if (!msg.GetPropStream(prop))
    {
        LOG_ERROR << "Error reading property stream from message" << std::endl;
        return;
    }
    std::string playerUuid;
    prop.ReadString(playerUuid);
#ifdef DEBUG_MATCH
    LOG_DEBUG << "Player " << playerUuid << " removing from queue" << std::endl;
#endif
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

void Application::ShowVersion()
{
    std::cout << SERVER_PRODUCT_NAME << " " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
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

bool Application::Initialize(const std::vector<std::string>& args)
{
    if (!ServerApp::Initialize(args))
        return false;

    if (!ParseCommandLine())
        return false;

    if (!sa::arg_parser::get_value<bool>(parsedArgs_, "nologo", false))
        ShowLogo();

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
    UpdateService(serv);
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
    serv.version = AB_SERVER_VERSION;
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
