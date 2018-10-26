#include "stdafx.h"
#include "Application.h"
#include "Subsystems.h"
#include "SimpleConfigManager.h"
#include "Logger.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"
#include "GetFileController.h"
#include "GetHTMLController.h"
#include "Profiler.h"
#include "Sessions.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    startTime_(0),
    dataPort_(0),
    adminPort_(0),
    ioService_()
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<Sessions>();
}

Application::~Application()
{
    if (running_)
        Stop();
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
        else if (a.compare("-id") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                serverId_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -id" << std::endl;
        }
        else if (a.compare("-ip") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                adminIp_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -ip" << std::endl;
        }
        else if (a.compare("-host") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                adminHost_ = arguments_[i];
            }
            else
                LOG_WARNING << "Missing argument for -host" << std::endl;
        }
        else if (a.compare("-port") == 0)
        {
            if (i + 1 < arguments_.size())
            {
                ++i;
                adminPort_ = static_cast<uint16_t>(atoi(arguments_[i].c_str()));
            }
            else
                LOG_WARNING << "Missing argument for -port" << std::endl;
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
    std::cout << "absadmin [-<option> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  id <id>: Server ID" << std::endl;
    std::cout << "  ip <ip>: Admin IP" << std::endl;
    std::cout << "  host <host>: Admin Host" << std::endl;
    std::cout << "  port <port>: Admin Port" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

void Application::PrintServerInfo()
{
    auto config = GetSubsystem<IO::SimpleConfigManager>();
    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << serverId_ << std::endl;
    LOG_INFO << "  Location: " << config->GetGlobal("location", "--") << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << (adminIp_.empty() ? "0.0.0.0" : adminIp_) << ":" << adminPort_ << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Worker Threads: " << server_->config.thread_pool_size << std::endl;
    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
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
        adminPort_ = static_cast<uint16_t>(config->GetGlobal("file_port", 8888));
    if (adminHost_.empty())
        adminHost_ = config->GetGlobal("admin_host", "");
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
    server_->default_resource["GET"] = std::bind(&Application::GetHandler, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->default_resource["POST"] = std::bind(&Application::PostHandler, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->on_error = std::bind(&Application::HandleError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    getController_["!!DEFAULT!!"] = std::make_unique<GetFileController>();
    getController_[".html"] = std::make_unique<GetHTMLController>();

    PrintServerInfo();

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

SimpleWeb::CaseInsensitiveMultimap Application::GetDefaultHeader()
{
    SimpleWeb::CaseInsensitiveMultimap result;
    result.emplace("Server", "absadmin");
    return result;
}

void Application::GetHandler(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    auto ext = Utils::GetFileExt(request->path);
    auto it = getController_.find(ext);
    if (it == getController_.end())
    {
        // Default file get controller
        it = getController_.find("!!DEFAULT!!");
    }
    if (it == getController_.end())
    {
        LOG_ERROR << "Unknown path " << request->path << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found", GetDefaultHeader());
        return;
    }
    AB_PROFILE;
    (*it).second->MakeRequest(response, request);
}

void Application::PostHandler(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    auto ext = Utils::GetFileExt(request->path);
    auto it = postController_.find(ext);
    if (it == postController_.end())
    {
        LOG_ERROR << "Unknown path " << request->path << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found", GetDefaultHeader());
        return;
    }
    AB_PROFILE;
    (*it).second->MakeRequest(response, request);
}

void Application::HandleError(std::shared_ptr<HttpsServer::Request>,
    const SimpleWeb::error_code& ec)
{
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    if (ec.value() == 995 || ec == SimpleWeb::errc::operation_canceled)
        return;

    LOG_ERROR << "(" << ec.value() << ") " << ec.message() << std::endl;
}
