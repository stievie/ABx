#include "stdafx.h"
#include "Application.h"
#include "Bridge.h"
#include "SimpleConfigManager.h"
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"
#include "Subsystems.h"
#include "BanManager.h"

Application::Application() :
    ServerApp::ServerApp(),
    ioService_(),
    lbType_(AB::Entities::ServiceTypeUnknown)
{
    serverType_ = AB::Entities::ServiceTypeLoadBalancer;
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
}

Application::~Application() = default;

void Application::ShowHelp()
{
    std::cout << "ablb [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

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
        configFile_ = path_ + "/" + "ablb_svc.lua";
#else
        configFile_ = path_ + "/" + "ablb.lua";
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
            if (Utils::TimeElapsed(s.heartbeat) > AB::Entities::HEARTBEAT_INTERVAL * 2)
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

    const auto& item = Utils::SelectRandomly(serviceList_.begin(), serviceList_.end());
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
        const std::vector<std::string> lineParts = Utils::Split(line, ":");
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
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = Utils::Tick();
    serv.heartbeat = Utils::Tick();
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    LOG_INFO << "Server is running" << std::endl;

    acceptor_->AcceptConnections();

    running_ = true;
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
        serv.stopTime = Utils::Tick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        dataClient_->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient_->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;
    ioService_.stop();
}
