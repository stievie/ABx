#include "stdafx.h"
#include "Application.h"
#include "Bridge.h"
#include "SimpleConfigManager.h"
#include <AB/Entities/ServiceList.h>

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
    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
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

    localHost_ = IO::SimpleConfigManager::Instance.GetGlobal("lb_host", "0.0.0.0");
    localPort_ = static_cast<uint16_t>(
        IO::SimpleConfigManager::Instance.GetGlobal("lb_port", 2700)
    );
    lbType_ = static_cast<AB::Entities::ServiceType>(
        IO::SimpleConfigManager::Instance.GetGlobal("lb_type", 4)
    );
    acceptor_ = std::make_unique<Acceptor>(ioService_, localHost_, localPort_,
        std::bind(&Application::GetServiceCallback, this, std::placeholders::_1));

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
    ioService_.stop();
}
