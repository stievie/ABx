#include "stdafx.h"
#include "Application.h"
#include "Subsystems.h"
#include "SimpleConfigManager.h"
#include "Logger.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    startTime_(0),
    dataPort_(0),
    adminHost_(0),
    ioService_()
{
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
}

Application::~Application()
{
    if (running_)
        Stop();
}

bool Application::Initialize(int argc, char** argv)
{
    if (!ServerApp::Initialize(argc, argv))
        return false;

    auto config = GetSubsystem<IO::SimpleConfigManager>();
    if (configFile_.empty())
        configFile_ = path_ + "/absadmin.lua";
    if (!config->Load(configFile_))
    {
        LOG_ERROR << "Error loading config file " << configFile_ << std::endl;
        return false;
    }

    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    if (serverId_.empty())
        serverId_ = config->GetGlobal("server_id", "00000000-0000-0000-0000-000000000000");
    if (adminIp_.empty())
        adminIp_ = config->GetGlobal("file_ip", "");
    if (adminPort_ == 0)
        adminPort_ = static_cast<uint16_t>(config->GetGlobal("file_port", 8081));
    std::string key = config->GetGlobal("server_key", "server.key");
    std::string cert = config->GetGlobal("server_cert", "server.crt");
    size_t threads = config->GetGlobal("num_threads", 0);
    if (threads == 0)
        threads = std::max<size_t>(1, std::thread::hardware_concurrency());
    root_ = config->GetGlobal("root_dir", "");
    logDir_ = config->GetGlobal("log_dir", "");
    dataHost_ = config->GetGlobal("data_host", "");
    dataPort_ = static_cast<uint16_t>(config->GetGlobal("data_port", 0));

    dataClient_ = std::make_unique<IO::DataClient>(ioService_);
    LOG_INFO << "Connecting to data server...";
    dataClient_->Connect(dataHost_, dataPort_);
    if (!dataClient_->IsConnected())
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    server_ = std::make_unique<HttpsServer>(cert, key);
    server_->config.port = adminPort_;
    if (!adminIp_.empty())
        server_->config.address = adminIp_;
    server_->config.thread_pool_size = threads;

    return true;
}

void Application::Run()
{
    auto config = GetSubsystem<IO::SimpleConfigManager>();
    startTime_ = Utils::AbTick();
    AB::Entities::Service serv;
    serv.uuid = serverId_;
    dataClient_->Read(serv);
    serv.name = config->GetGlobal("server_name", "absadmin");
    serv.location = config->GetGlobal("location", "--");
    serv.host = adminHost_;
    serv.port = adminPort_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeAdminServer;
    serv.startTime = startTime_;
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    server_->start();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown...";
    AB::Entities::Service serv;
    serv.uuid = serverId_;
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

    server_->stop();
}
