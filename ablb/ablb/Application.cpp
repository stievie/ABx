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
#include "Bridge.h"
#include "Version.h"
#include <AB/Entities/ServiceList.h>
#include <abscommon/BanManager.h>
#include <abscommon/Logo.h>
#include <abscommon/PingServer.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>
#include <sa/StringTempl.h>
#include <sa/Iterator.h>
#include <sa/time.h>

Application::Application() :
    ServerApp::ServerApp(),
    ioService_(),
    lbType_(AB::Entities::ServiceTypeUnknown)
{
    programDescription_ = SERVER_PRODUCT_NAME;
    serverType_ = AB::Entities::ServiceTypeLoadBalancer;
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    Subsystems::Instance.CreateSubsystem<Net::PingServer>();
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
}

Application::~Application() = default;

void Application::PrintServerInfo()
{
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Machine: " << machine_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << serverHost_ << ":" << static_cast<int>(serverPort_) << std::endl;
    if (dataClient_->IsConnected())
        LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
    else
    {
        LOG_INFO << "  Upstreams: ";
        for (const auto& item : serviceList_)
        {
            LOG_INFO << item.first << ":" << item.second << " ";
        }
        LOG_INFO << std::endl;
    }
}

bool Application::LoadMain()
{
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = Utils::ConcatPath(path_, "ablb_svc.lua");
#else
        configFile_ = Utils::ConcatPath(path_, "ablb.lua");
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
        serverName_ = config->GetGlobalString("server_name", "ablb");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");
    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");

    uint16_t dataPort = static_cast<uint16_t>(config->GetGlobalInt("data_port", 0));
    if (dataPort != 0)
    {
        LOG_INFO << "Connecting to data server...";
        const std::string dataHost = config->GetGlobalString("data_host", "");
        dataClient_->Connect(dataHost, dataPort);
        if (!dataClient_->IsConnected())
        {
            LOG_INFO << "[FAIL]" << std::endl;
            LOG_ERROR << "Failed to connect to data server" << std::endl;
            return false;
        }
        LOG_INFO << "[done]" << std::endl;
        if (serverName_.empty() || serverName_.compare("generic") == 0)
        {
            serverName_ = GetFreeName(dataClient_.get());
        }
    }
    else
    {
        const std::string serverList = config->GetGlobalString("server_list", "");
        if (!ParseServerList(serverList))
        {
            LOG_ERROR << "Error parsing server list file " << serverList << std::endl;
            return false;
        }
    }

    if (serverIp_.empty())
        serverHost_ = config->GetGlobalString("lb_ip", "0.0.0.0");
    if (serverHost_.empty())
        serverHost_ = config->GetGlobalString("lb_host", "0.0.0.0");
    if (serverPort_ == std::numeric_limits<uint16_t>::max())
    {
        serverPort_ = static_cast<uint16_t>(config->GetGlobalInt("lb_port", 2740));
    }
    lbType_ = static_cast<AB::Entities::ServiceType>(
        // Default is login server
        config->GetGlobalInt("lb_type", static_cast<int64_t>(AB::Entities::ServiceTypeLoginServer))
    );
    if (dataPort != 0)
        // We have a data port so we can query the data server
        acceptor_ = std::make_unique<Acceptor>(ioService_, serverHost_, serverPort_,
            std::bind(&Application::GetServiceCallback, this, std::placeholders::_1));
    else
        // Get service list from config file
        acceptor_ = std::make_unique<Acceptor>(ioService_, serverHost_, serverPort_,
            std::bind(&Application::GetServiceCallbackList, this, std::placeholders::_1));

    GetSubsystem<Net::PingServer>()->port_ = serverPort_;

    PrintServerInfo();
    return true;
}

bool Application::GetServiceCallback(AB::Entities::Service& svc)
{
    AB::Entities::ServiceList sl;
    if (!dataClient_->Read(sl))
        return false;

    std::vector<AB::Entities::Service> services;

    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dataClient_->Read(s))
            continue;
        if (s.status != AB::Entities::ServiceStatusOnline)
            continue;
        if (s.type == AB::Entities::ServiceTypeFileServer ||
            s.type == AB::Entities::ServiceTypeGameServer ||
            s.type == AB::Entities::ServiceTypeLoginServer)
        {
            if (sa::time::time_elapsed(s.heartbeat) > AB::Entities::HEARTBEAT_INTERVAL * 2)
                // Maybe dead
                continue;
        }
        if (s.type == lbType_)
        {
            services.push_back(s);
        }
    }

    if (services.size() != 0)
    {
        std::sort(services.begin(), services.end(),
            [](AB::Entities::Service const& a, AB::Entities::Service const& b)
        {
            return a.load < b.load;
        });
        if (services[0].type == AB::Entities::ServiceTypeFileServer || services[0].load < 100)
        {
            svc = services[0];
            return true;
        }
    }

    LOG_WARNING << "No server of type " << static_cast<int>(lbType_) << " online" << std::endl;
    return false;
}

bool Application::GetServiceCallbackList(AB::Entities::Service& svc)
{
    if (serviceList_.size() == 0)
    {
        LOG_WARNING << "Service list is empty" << std::endl;
        return false;
    }

    const auto item = sa::SelectRandomly(serviceList_.begin(), serviceList_.end());
    svc.host = (*item).first;
    svc.port = (*item).second;
    return true;
}

bool Application::ParseServerList(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        LOG_ERROR << "Unable to open file " << fileName << std::endl;
        return false;
    }
    std::string line;
    // <host>:<port>\n
    while (std::getline(file, line))
    {
        const std::vector<std::string> lineParts = sa::Split(line, ":");
        if (lineParts.size() == 2)
        {
            serviceList_.push_back({
                lineParts[0],
                static_cast<uint16_t>(std::atoi(lineParts[1].c_str()))
            });
        }
        else
        {
            LOG_WARNING << "Error: Config line skipped: " << line << std::endl;
        }
    }
    return true;
}

void Application::ShowVersion()
{
    std::cout << SERVER_PRODUCT_NAME << " " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR;
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

    return true;
}

void Application::Run()
{
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient_->Read(serv);
    serv.location = serverLocation_;
    serv.host = serverHost_;
    serv.port = serverPort_;
    serv.ip = serverIp_;
    serv.name = serverName_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = sa::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = sa::time::tick();
    serv.heartbeat = serv.startTime;
    serv.version = AB_SERVER_VERSION;
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    LOG_INFO << "Server is running" << std::endl;

    acceptor_->AcceptConnections();

    running_ = true;
    if (lbType_ == AB::Entities::ServiceTypeLoginServer)
        GetSubsystem<Net::PingServer>()->Start();
    ioService_.run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;

    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (dataClient_->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = sa::time::tick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        dataClient_->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient_->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;
    if (lbType_ == AB::Entities::ServiceTypeLoginServer)
        GetSubsystem<Net::PingServer>()->Stop();
    ioService_.stop();
}
