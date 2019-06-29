#include "stdafx.h"
#include "Application.h"
#include "Subsystems.h"
#include "SimpleConfigManager.h"
#include "BanManager.h"
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
#include "AccountKeysResource.h"
#include "UpdateAccountKeyResource.h"
#include "CreateKeyResource.h"
#include "LessFileResource.h"
#include "AccountsResource.h"
#include "AccountsJsonResource.h"
#include "ClearCacheResource.h"
#include "AccountResource.h"
#include "AccountPostResource.h"
#include "AccountLogoutResource.h"
#include "DownloadResource.h"

Application* Application::Instance = nullptr;

Application::Application() :
    ServerApp::ServerApp(),
    startTime_(0)
{
    assert(Application::Instance == nullptr);
    Application::Instance = this;

    serverType_ = AB::Entities::ServiceTypeAdminServer;

    ioService_ = std::make_shared<asio::io_service>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<HTTP::Sessions>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(*ioService_);
    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    Subsystems::Instance.CreateSubsystem<ContentTypes>();
    Subsystems::Instance.CreateSubsystem<Net::MessageClient>(*ioService_);
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
    std::stringstream url;
    url << "https://";
    const SimpleWeb::CaseInsensitiveMultimap& _header = request->header;
    auto it = _header.find("Host");
    if (it == _header.end())
    {
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
            "Internal Server Error");
        return;
    }
    auto hHeader = (*it).second;
    url << hHeader;
    if (server_->config.port != 443)
        url << ":" << std::to_string(server_->config.port);
    url << request->path;
    if (!request->query_string.empty())
        url << "?" << request->query_string;
    header.emplace("Location", url.str());
    response->write(SimpleWeb::StatusCode::redirection_moved_permanently, "Moved Permanently", header);
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
    auto* dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << (serverIp_.empty() ? "0.0.0.0" : serverIp_) << ":" << serverPort_ << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Worker Threads: " << server_->config.thread_pool_size << std::endl;
    LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
}

void Application::HandleMessage(const Net::MessageMsg&)
{
    // Nothing to handle
}

void Application::InitContentTypes()
{
    // https://www.freeformatter.com/mime-types-list.html
    auto* conT = GetSubsystem<ContentTypes>();
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
}

void Application::InitRoutes()
{
    DefaultRoute<Resources::FileResource>("GET");
    Route<Resources::LessFileResource>("GET", "^/.+\\.less$");
    Route<Resources::IndexResource>("GET", "^/$");
    Route<Resources::ServicesResource>("GET", "^/services$");
    Route<Resources::ServiceResource>("GET", "^/service$");
    Route<Resources::ServicesJsonResource>("GET", "^/get/services$");
    Route<Resources::ProfileResource>("GET", "^/profile$");
    Route<Resources::FriendsResource>("GET", "^/friends$");
    Route<Resources::AccountsResource>("GET", "^/accounts$");
    Route<Resources::AccountsJsonResource>("GET", "^/get/accounts$");
    Route<Resources::AccountResource>("GET", "^/account$");
    Route<Resources::AccountKeysResource>("GET", "^/accountkeys$");
    Route<Resources::DownloadResource>("GET", "^/download$");

    Route<Resources::LoginResource>("POST", "^/post/login$");
    Route<Resources::LogoutResource>("POST", "^/post/logout$");
    Route<Resources::ProfilePostResource>("POST", "^/post/profile$");
    Route<Resources::PasswordPostResource>("POST", "^/post/password$");
    Route<Resources::SpawnResource>("POST", "^/post/spawn$");
    Route<Resources::TerminateResource>("POST", "^/post/terminate$");
    Route<Resources::ClearCacheResource>("POST", "^/post/clear_cache$");
    Route<Resources::UpdateAccountKeyResource>("POST", "^/post/updateaccountkey$");
    Route<Resources::CreateKeyResource>("POST", "^/post/createkey$");
    Route<Resources::AccountPostResource>("POST", "^/post/account$");
    Route<Resources::AccountLogoutResource>("POST", "^/post/account_logout$");
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

    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    LOG_INFO << "Loading configuration...";
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = path_ + "/" + "absadmin_svc.lua";
#else
        configFile_ = path_ + "/" + "absadmin.lua";
#endif
    }

    if (!config->Load(configFile_))
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_ERROR << "Error loading config file " << configFile_ << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");
    if (!logDir_.empty())
    {
        // Different log dir
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    if (serverId_.empty() || uuids::uuid(serverId_).nil())
        serverId_ = config->GetGlobalString("server_id", Utils::Uuid::EMPTY_UUID);
    if (serverName_.empty())
        serverName_ = config->GetGlobalString("server_name", "absadmin");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");

    if (serverIp_.empty())
        serverIp_ = config->GetGlobalString("admin_ip", "");
    if (serverPort_ == std::numeric_limits<uint16_t>::max())
        serverPort_ = static_cast<uint16_t>(config->GetGlobalInt("admin_port", 443ll));
    if (serverHost_.empty())
        serverHost_ = config->GetGlobalString("admin_host", "");
    HTTP::Session::sessionLifetime_ = static_cast<uint32_t>(config->GetGlobalInt("session_lifetime", (int64_t)HTTP::Session::sessionLifetime_));
    std::string key = config->GetGlobalString("server_key", "server.key");
    std::string cert = config->GetGlobalString("server_cert", "server.crt");
    size_t threads = config->GetGlobalInt("num_threads", 0ll);
    if (threads == 0)
        threads = std::max<size_t>(1, std::thread::hardware_concurrency());
    root_ = config->GetGlobalString("root_dir", "");
    std::string dataHost = config->GetGlobalString("data_host", "");
    uint16_t dataPort = static_cast<uint16_t>(config->GetGlobalInt("data_port", 0ll));

    Auth::BanManager::LoginTries = static_cast<uint32_t>(config->GetGlobalInt("login_tries", 5ll));
    Auth::BanManager::LoginRetryTimeout = static_cast<uint32_t>(config->GetGlobalInt("login_retrytimeout", 5000ll));

    auto* dataClient = GetSubsystem<IO::DataClient>();
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

    std::string msgHost = config->GetGlobalString("message_host", "");
    uint16_t msgPort = static_cast<uint16_t>(config->GetGlobalInt("message_port", 0ll));
    auto* msgClient = GetSubsystem<Net::MessageClient>();
    LOG_INFO << "Connecting to message server...";
    msgClient->Connect(msgHost, msgPort, std::bind(&Application::HandleMessage, this, std::placeholders::_1));
    if (msgClient->IsConnected())
        LOG_INFO << "[done]" << std::endl;
    else
    {
        LOG_INFO << "[FAIL]" << std::endl;
        LOG_WARNING << "Not connected to message server" << std::endl;
    }

    // Redirect to HTTPS
    httpServer_ = std::make_unique<HttpServer>();
    httpServer_->config.port = 80;
    if (!serverIp_.empty())
        httpServer_->config.address = serverIp_;
    httpServer_->io_service = ioService_;
    httpServer_->on_accept = std::bind(&Application::HandleOnAccept, shared_from_this(),
        std::placeholders::_1);
    httpServer_->default_resource["GET"] = std::bind(&Application::HtttpsRedirect, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    try
    {
        server_ = std::make_unique<HttpsServer>(cert, key);
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR << ex.what() << std::endl;
        return false;
    }
    server_->config.port = serverPort_;
    if (!serverIp_.empty())
        server_->config.address = serverIp_;
    server_->config.thread_pool_size = threads;
    server_->on_error = std::bind(&Application::HandleError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->on_accept = std::bind(&Application::HandleOnAccept, shared_from_this(),
        std::placeholders::_1);
    server_->io_service = ioService_;

    InitContentTypes();
    InitRoutes();

    PrintServerInfo();

    return true;
}

void Application::Run()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    startTime_ = Utils::Tick();
    AB::Entities::Service serv;
    serv.uuid = GetServerId();
    dataClient->Read(serv);
    serv.name = serverName_;
    serv.location = serverLocation_;
    serv.host = serverHost_;
    serv.port = serverPort_;
    serv.ip = serverIp_;
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = serverType_;
    serv.startTime = startTime_;
    serv.heardbeat = Utils::Tick();
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
    auto* dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Server shutdown..." << std::endl;
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
    if (ec.default_error_condition().value() == 995 || ec == SimpleWeb::errc::operation_canceled)
        return;

    LOG_ERROR << "(" << ec.default_error_condition().value() << ") " << ec.default_error_condition().message() << std::endl;
}

bool Application::HandleOnAccept(const asio::ip::tcp::endpoint& endpoint)
{
    auto* banMan = GetSubsystem<Auth::BanManager>();
    return banMan->AcceptConnection(endpoint.address().to_v4().to_ulong());
}
