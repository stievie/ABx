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
#include "ProtocolLogin.h"
#include "Version.h"
#include <AB/DHKeys.hpp>
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include <abscommon/BanManager.h>
#include <abscommon/Connection.h>
#include <abscommon/DataClient.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Logo.h>
#include <abscommon/OutputMessage.h>
#include <abscommon/PingServer.h>
#include <abscommon/Random.h>
#include <abscommon/Scheduler.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>

Application::Application() :
    ServerApp::ServerApp(),
    ioService_(std::make_shared<asio::io_service>())
{
    programDescription_ = SERVER_PRODUCT_NAME;
    serverType_ = AB::Entities::ServiceTypeLoginServer;

    static constexpr size_t EXPECTED_CONNECTIONS = 128;
    static constexpr size_t NETWORKMESSAGE_POOLCOUNT = EXPECTED_CONNECTIONS * 2;
    static constexpr size_t NETWORKMESSAGE_POOLSIZE = Net::NETWORKMESSAGE_MAXSIZE * NETWORKMESSAGE_POOLCOUNT;
    static constexpr size_t OUTPUTMESSAGE_POOLCOUNT = EXPECTED_CONNECTIONS * 2;
    static constexpr size_t OUTPUTMESSAGE_POOLSIZE = Net::OUTPUTMESSAGE_SIZE * OUTPUTMESSAGE_POOLCOUNT;

    // Need for the Connection object
    Subsystems::Instance.CreateSubsystem<Net::NetworkMessage::MessagePool>(NETWORKMESSAGE_POOLSIZE);
    // OutputMessage pool
    Subsystems::Instance.CreateSubsystem<Net::PoolWrapper::MessagePool>(OUTPUTMESSAGE_POOLSIZE);
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<Net::ConnectionManager>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(*ioService_);
    Subsystems::Instance.CreateSubsystem<Net::MessageClient>(*ioService_);
    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    Subsystems::Instance.CreateSubsystem<Crypto::Random>();
    Subsystems::Instance.CreateSubsystem<Crypto::DHKeys>();
    Subsystems::Instance.CreateSubsystem<Net::PingServer>();

    serviceManager_ = std::make_unique<Net::ServiceManager>(*ioService_);
}

Application::~Application()
{
    serviceManager_->Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
    GetSubsystem<Net::ConnectionManager>()->CloseAll();
}

bool Application::LoadMain()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = Utils::ConcatPath(path_, "ablogin_svc.lua");
#else
        configFile_ = Utils::ConcatPath(path_, "ablogin.lua");
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
            bool ret = GetSubsystem<Auth::BanManager>()->AcceptConnection(remoteIp);
            if (!ret)
                LOG_WARNING << "Not accepting connection from " << Utils::ConvertIPToString(remoteIp) << std::endl;
            return ret;
        }))
            return false;
    }
    else
    {
        LOG_ERROR << "Port can not be 0" << std::endl;
        return false;
    }
    GetSubsystem<Net::PingServer>()->port_ = serverPort_;

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
    LOG_INFO << "  Message Server: " << msgClient->GetHost() << ":" << msgClient->GetPort() << std::endl;
}

void Application::HeartBeatTask()
{
#ifdef DEBUG_POOLALLOCATOR
    // Print some stats
    sa::PoolInfo info = Net::OutputMessagePool::GetPoolInfo();
    LOG_DEBUG << "OutputMessage Pool:  allocs: " << info.allocs << ", frees: " << info.frees << ", current: " << info.current <<
        ", peak: " << info.peak <<
        ", used: " << info.used << ", avail: " << info.avail << std::endl;
#endif

    auto* dataClient = GetSubsystem<IO::DataClient>();
    if (dataClient->IsConnected())
    {
        AB::Entities::Service serv;
        serv.uuid = serverId_;
        if (dataClient->Read(serv))
        {
            serv.heartbeat = Utils::Tick();
            if (!dataClient->Update(serv))
                LOG_ERROR << "Error updating service " << serverId_ << std::endl;
        }
        else
            LOG_ERROR << "Error reading service " << serverId_ << std::endl;
    }
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(AB::Entities::HEARTBEAT_INTERVAL, std::bind(&Application::HeartBeatTask, this))
        );
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

void Application::HandleMessage(const Net::MessageMsg&)
{
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
    UpdateService(serv);
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
    serv.version = AB_SERVER_VERSION;
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(AB::Entities::HEARTBEAT_INTERVAL, std::bind(&Application::HeartBeatTask, this))
    );
    // If we want to receive messages, we need to send our ServerID to the message server.
    SendServerJoined(GetSubsystem<Net::MessageClient>(), serv);

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    serviceManager_->Run();
    GetSubsystem<Net::PingServer>()->Start();
    ioService_->run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;

    GetSubsystem<Net::PingServer>()->Stop();

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

    ioService_->stop();
}

std::string Application::GetKeysFile() const
{
    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    const std::string keys = config->GetGlobalString("server_keys", "");
    if (!keys.empty())
        return keys;
    return Utils::AddSlash(path_) + "abserver.dh";
}
