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

#include "stdafx.h"
#include "Application.h"
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

bool Application::LoadMain()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = Utils::ConcatPath(path_, "abmsgs_svc.lua");
#else
        configFile_ = Utils::ConcatPath(path_, "abmsgs.lua");
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
        serverName_ = config->GetGlobalString("server_name", "abmsgs");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");
    if (serverHost_.empty())
        serverHost_ = config->GetGlobalString("message_host", "");
    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");
    const std::string ips = config->GetGlobalString("allowed_ips", "");
    whiteList_.AddList(ips);

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
    if (serverName_.empty() || serverName_.compare("generic") == 0)
    {
        serverName_ = GetFreeName(dataClient);
    }

    // Add Protocols
    if (serverIp_.empty())
        serverIp_ = config->GetGlobalString("message_ip", "0.0.0.0");
    if (serverPort_ == std::numeric_limits<uint16_t>::max())
        serverPort_ = static_cast<uint16_t>(config->GetGlobalInt("message_port", 2771ll));

    PrintServerInfo();
    return true;
}

void Application::PrintServerInfo()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Machine: " << machine_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;

    LOG_INFO << "  Listening: ";
    LOG_INFO << serverIp_ << ":" << static_cast<int>(serverPort_) << std::endl;

    LOG_INFO << "  Allowed IPs: ";
    if (whiteList_.IsEmpty())
        LOG_INFO << "(all)";
    else
        LOG_INFO << whiteList_.ToString();
    LOG_INFO << std::endl;

    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
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
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

    uint32_t ip = Utils::ConvertStringToIP(serverIp_);
    asio::ip::tcp::endpoint endpoint(asio::ip::address(asio::ip::address_v4(ip)), serverPort_);
    server_ = std::make_unique<MessageServer>(ioService_, endpoint, whiteList_);

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
