#include "stdafx.h"
#include "Application.h"
#include "Subsystems.h"
#include "SimpleConfigManager.h"
#include "Logger.h"
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include "StringUtils.h"
#include "Profiler.h"
#include "Sessions.h"
#include "DataClient.h"
#include "FileResource.h"
#include "IndexResource.h"
#include "ContentTypes.h"
#include "LoginResource.h"
#include "LogoutResource.h"
#include "ServicesResource.h"
#include "ServicesJsonResource.h"
#include "ProfileResource.h"
#include "ProfilePostResource.h"
#include "PasswordPostResource.h"
#include "FriendsResource.h"
#include "ServiceResource.h"
#include "SpawnResource.h"
#include "TerminateResource.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    startTime_(0),
    adminPort_(0)
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;

    serverType_ = AB::Entities::ServiceTypeAdminServer;

    ioService_ = std::make_shared<asio::io_service>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<HTTP::Sessions>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(*ioService_.get());
    Subsystems::Instance.CreateSubsystem<ContentTypes>();
    Subsystems::Instance.CreateSubsystem<Net::MessageClient>(*ioService_.get());
}

Application::~Application()
{
    if (running_)
        Stop();
}

void Application::HtttpsRedirect(std::shared_ptr<HttpServer::Response> response,
    std::shared_ptr<HttpServer::Request> request)
{
    auto header = GetDefaultHeader();
    std::string url = "https://";
    const SimpleWeb::CaseInsensitiveMultimap& _header = request->header;
    auto it = _header.find("Host");
    if (it == _header.end())
    {
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
            "Internal Server Error");
        return;
    }
    auto hHeader = (*it).second;
    url += hHeader;
    if (server_->config.port != 443)
        url += ":" + std::to_string(server_->config.port);
    url += request->path;
    if (!request->query_string.empty())
        url += "?" + request->query_string;
    header.emplace("Location", url);
    response->write(SimpleWeb::StatusCode::redirection_moved_permanently, "Moved Permanently", header);
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
    auto dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << (adminIp_.empty() ? "0.0.0.0" : adminIp_) << ":" << adminPort_ << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Worker Threads: " << server_->config.thread_pool_size << std::endl;
    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
}

void Application::HandleMessage(const Net::MessageMsg&)
{
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
    LOG_INFO << "Loading configuration...";
    if (configFile_.empty())
        configFile_ = path_ + "/absadmin.lua";
    if (!config->Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Error loading config file " << configFile_ << std::endl;
        return false;
    }

    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    if (serverId_.empty() || uuids::uuid(serverId_).nil())
        serverId_ = config->GetGlobal("server_id", Utils::Uuid::EMPTY_UUID);
    if (serverName_.empty())
        serverName_ = config->GetGlobal("server_name", "absadmin");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobal("location", "--");

    if (adminIp_.empty())
        adminIp_ = config->GetGlobal("admin_ip", "");
    if (adminPort_ == 0)
        adminPort_ = static_cast<uint16_t>(config->GetGlobal("admin_port", 443));
    if (adminHost_.empty())
        adminHost_ = config->GetGlobal("admin_host", "");
    HTTP::Session::sessionLifetime_ = static_cast<uint32_t>(config->GetGlobal("session_lifetime", HTTP::Session::sessionLifetime_));
    std::string key = config->GetGlobal("server_key", "server.key");
    std::string cert = config->GetGlobal("server_cert", "server.crt");
    size_t threads = config->GetGlobal("num_threads", 0);
    if (threads == 0)
        threads = std::max<size_t>(1, std::thread::hardware_concurrency());
    root_ = config->GetGlobal("root_dir", "");
    logDir_ = config->GetGlobal("log_dir", "");
    std::string dataHost = config->GetGlobal("data_host", "");
    uint16_t dataPort = static_cast<uint16_t>(config->GetGlobal("data_port", 0));
    LOG_INFO << "[done]" << std::endl;

    auto dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Connecting to data server...";
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

    std::string msgHost = config->GetGlobal("message_host", "");
    uint16_t msgPort = static_cast<uint16_t>(config->GetGlobal("message_port", 0));
    auto msgClient = GetSubsystem<Net::MessageClient>();
    LOG_INFO << "Connecting to message server...";
    msgClient->Connect(msgHost, msgPort, std::bind(&Application::HandleMessage, this, std::placeholders::_1));
    if (msgClient->IsConnected())
        LOG_INFO << "[done]" << std::endl;
    else
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_WARNING << "Not connected to message server" << std::endl;
    }

    // https://www.freeformatter.com/mime-types-list.html
    auto conT = GetSubsystem<ContentTypes>();
    conT->map_[".css"] = "text/css";
    conT->map_[".html"] = "text/html";
    conT->map_[".js"] = "application/javascript";
    conT->map_[".json"] = "application/json";
    conT->map_[".pdf"] = "application/pdf";
    conT->map_[".zip"] = "application/zip";
    conT->map_[".gif"] = "image/gif";
    conT->map_[".jpg"] = "image/jpeg";
    conT->map_[".jpeg"] = "image/jpeg";
    conT->map_[".png"] = "image/png";
    conT->map_[".svg"] = "image/svg+xml";
    conT->map_[".ico"] = "image/x-icon";
    conT->map_[".otf"] = "font/otf";
    conT->map_[".sfnt"] = "font/sfnt";
    conT->map_[".ttf"] = "font/ttf";
    conT->map_[".woff"] = "font/woff";
    conT->map_[".woff2"] = "font/woff2";

    // Redirect to HTTPS
    httpServer_ = std::make_unique<HttpServer>();
    httpServer_->config.port = 80;
    if (!adminIp_.empty())
        httpServer_->config.address = adminIp_;
    httpServer_->io_service = ioService_;
    httpServer_->default_resource["GET"] = std::bind(&Application::HtttpsRedirect, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    server_ = std::make_unique<HttpsServer>(cert, key);
    server_->config.port = adminPort_;
    if (!adminIp_.empty())
        server_->config.address = adminIp_;
    server_->config.thread_pool_size = threads;
    server_->on_error = std::bind(&Application::HandleError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->io_service = ioService_;

    DefaultRoute<Resources::FileResource>("GET");
    Route<Resources::IndexResource>("GET", "^/$");
    Route<Resources::ServicesResource>("GET", "^/services$");
    Route<Resources::ServiceResource>("GET", "^/service$");
    Route<Resources::ServicesJsonResource>("GET", "^/get/services$");
    Route<Resources::ProfileResource>("GET", "^/profile$");
    Route<Resources::FriendsResource>("GET", "^/friends$");
    Route<Resources::LoginResource>("POST", "^/post/login$");
    Route<Resources::LogoutResource>("POST", "^/post/logout$");
    Route<Resources::ProfilePostResource>("POST", "^/post/profile$");
    Route<Resources::PasswordPostResource>("POST", "^/post/password$");
    Route<Resources::SpawnResource>("POST", "^/post/spawn$");
    Route<Resources::TerminateResource>("POST", "^/post/terminate");

    PrintServerInfo();

    return true;
}

void Application::Run()
{
    auto dataClient = GetSubsystem<IO::DataClient>();
    startTime_ = Utils::AbTick();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient->Read(serv);
    serv.name = serverName_;
    serv.location = serverLocation_;
    serv.host = adminHost_;
    serv.port = adminPort_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = startTime_;
    dataClient->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient->Invalidate(sl);

    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    server_->start();
    httpServer_->start();
    ioService_->run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    auto dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server shutdown...";
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    if (dataClient->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = Utils::AbTick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;
        dataClient->Update(serv);

        AB::Entities::ServiceList sl;
        dataClient->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;

    httpServer_->stop();
    server_->stop();
    ioService_->stop();
}

SimpleWeb::CaseInsensitiveMultimap Application::GetDefaultHeader()
{
    assert(Application::Instance);
    SimpleWeb::CaseInsensitiveMultimap result;
    result.emplace("Server", Application::Instance->serverName_);
    return result;
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
