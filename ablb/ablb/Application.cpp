#include "stdafx.h"
#include "Application.h"
#include "Bridge.h"
#include "SimpleConfigManager.h"
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"
#include "Utils.h"

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    ioService_()
{
    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
}

Application::~Application()
{
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
    std::cout << "ablb [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

void Application::PrintServerInfo()
{
    LOG_INFO << "Server Info:" << std::endl;
    LOG_INFO << "  Server ID: " << IO::SimpleConfigManager::Instance.GetGlobal("server_id", "") << std::endl;
    LOG_INFO << "  Location: " << IO::SimpleConfigManager::Instance.GetGlobal("location", "--") << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << localHost_ << ":" << static_cast<int>(localPort_) << std::endl;
    if (dataClient_->IsConnected())
        LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
    else
    {
        LOG_INFO << "  Upstream: ";
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
        configFile_ = path_ + "/ablb.lua";

    LOG_INFO << "Loading configuration...";
    if (!IO::SimpleConfigManager::Instance.Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        return false;
    }

    uint16_t dataPort = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("data_port", 0));
    if (dataPort != 0)
    {
        LOG_INFO << "Connecting to data server...";
        const std::string dataHost = IO::SimpleConfigManager::Instance.GetGlobal("data_host", "");
        dataClient_->Connect(dataHost, dataPort);
        if (!dataClient_->IsConnected())
        {
            LOG_INFO << "[FAIL]" << std::endl;
            LOG_ERROR << "Failed to connect to data server" << std::endl;
            return false;
        }
        LOG_INFO << "[done]" << std::endl;
    }
    else
    {
        const std::string serverList = IO::SimpleConfigManager::Instance.GetGlobal("server_list", "");
        if (!ParseServerList(serverList))
        {
            LOG_ERROR << "Error parsing server list file " << serverList << std::endl;
            return false;
        }
    }

    localHost_ = IO::SimpleConfigManager::Instance.GetGlobal("lb_host", "0.0.0.0");
    localPort_ = static_cast<uint16_t>(
        IO::SimpleConfigManager::Instance.GetGlobal("lb_port", 2740)
    );
    lbType_ = static_cast<AB::Entities::ServiceType>(
        // Default is login server
        IO::SimpleConfigManager::Instance.GetGlobal("lb_type", 4)
    );
    if (dataPort != 0)
        acceptor_ = std::make_unique<Acceptor>(ioService_, localHost_, localPort_,
            std::bind(&Application::GetServiceCallback, this, std::placeholders::_1));
    else
        acceptor_ = std::make_unique<Acceptor>(ioService_, localHost_, localPort_,
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

    const auto& item = Utils::select_randomly(serviceList_.begin(), serviceList_.end());
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
    serv.host = localHost_;
    serv.port = localPort_;
    serv.name = IO::SimpleConfigManager::Instance.GetGlobal("server_name", "ablb");
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeLoadBalancer;
    serv.startTime = Utils::AbTick();
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
    LOG_INFO << "Server shutdown...";

    AB::Entities::Service serv;
    serv.uuid = IO::SimpleConfigManager::Instance.GetGlobal("server_id", "");
    if (dataClient_->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::AbTick();
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
