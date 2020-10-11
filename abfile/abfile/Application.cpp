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
#include "Version.h"
#include <algorithm>
#include <sa/StringTempl.h>
#include <sa/time.h>
#include <AB/Entities/AccountBan.h>
#include <AB/Entities/Attribute.h>
#include <AB/Entities/AttributeList.h>
#include <AB/Entities/Ban.h>
#include <AB/Entities/Effect.h>
#include <AB/Entities/EffectList.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/GameList.h>
#include <AB/Entities/IpBan.h>
#include <AB/Entities/Item.h>
#include <AB/Entities/ItemList.h>
#include <AB/Entities/Music.h>
#include <AB/Entities/MusicList.h>
#include <AB/Entities/News.h>
#include <AB/Entities/NewsList.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/ProfessionList.h>
#include <AB/Entities/Quest.h>
#include <AB/Entities/QuestList.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Skill.h>
#include <AB/Entities/SkillList.h>
#include <AB/Entities/Version.h>
#include <AB/Entities/VersionList.h>
#include <abscommon/BanManager.h>
#include <abscommon/DataClient.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Logger.h>
#include <abscommon/Logo.h>
#include <abscommon/Scheduler.h>
#include <abscommon/Service.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>
#include <abscommon/Xml.h>
#include <fstream>
#include <sstream>

Application::Application() :
    ServerApp::ServerApp(),
    requireAuth_(false),
    startTime_(0),
    bytesSent_(0),
    uptimeRound_(0),
    statusMeasureTime_(0),
    lastLoadCalc_(0),
    temporary_(false)
{
    programDescription_ = SERVER_PRODUCT_NAME;
    serverType_ = AB::Entities::ServiceTypeFileServer;
    ioService_ = std::make_shared<asio::io_service>();
    Subsystems::Instance.CreateSubsystem<Asynch::Dispatcher>();
    Subsystems::Instance.CreateSubsystem<Asynch::Scheduler>();
    Subsystems::Instance.CreateSubsystem<IO::SimpleConfigManager>();
    Subsystems::Instance.CreateSubsystem<IO::DataClient>(*ioService_);
    Subsystems::Instance.CreateSubsystem<Auth::BanManager>();
    Subsystems::Instance.CreateSubsystem<Net::MessageClient>(*ioService_);
    cli_.push_back({ "temp", { "-temp", "--temporary" }, "Temporary application", false, false, sa::arg_parser::option_type::none });
}

Application::~Application()
{
    if (running_)
        Stop();
    GetSubsystem<Asynch::Scheduler>()->Stop();
    GetSubsystem<Asynch::Dispatcher>()->Stop();
}

void Application::HandleMessage(const Net::MessageMsg& msg)
{
    switch (msg.type_)
    {
    case Net::MessageType::Shutdown:
    {
        std::string serverId = msg.GetBodyString();
        if (Utils::Uuid::IsEqual(serverId, serverId_))
            Stop();
        break;
    }
    case Net::MessageType::Spawn:
        Spawn("-temp");
        break;
    default:
        break;
    }
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

void Application::ShowVersion()
{
    std::cout << SERVER_PRODUCT_NAME << " " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR << std::endl;
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
}

bool Application::ParseCommandLine()
{
    if (!ServerApp::ParseCommandLine())
        return false;

    if (sa::arg_parser::get_value<bool>(parsedArgs_, "temp", false))
        temporary_ = true;
    return true;
}

void Application::UpdateBytesSent(size_t bytes)
{
    std::scoped_lock lock(mutex_);
    if (Utils::WouldExceed(bytesSent_, bytes, std::numeric_limits<size_t>::max()))
    {
        bytesSent_ = 0;
        statusMeasureTime_ = sa::time::tick();
        ++uptimeRound_;
    }
    bytesSent_ += bytes;

    // Calculate load
    if (sa::time::time_elapsed(lastLoadCalc_) > 1000 || loads_.IsEmpty())
    {
        lastLoadCalc_ = sa::time::tick();

        unsigned load = 0;
        if (maxThroughput_ != 0)
        {
            uint64_t mesTime = sa::time::time_elapsed(statusMeasureTime_);
            int bytesPerSecond = static_cast<int>(bytesSent_ / (mesTime / 1000));
            float ld = (static_cast<float>(bytesPerSecond) / static_cast<float>(maxThroughput_)) * 100.0f;
            load = static_cast<unsigned>(ld);
        }
        loads_.Enqueue(std::min(load, 100u));
    }
}

void Application::HeartBeatTask()
{
    auto* dataClient = GetSubsystem<IO::DataClient>();
    if (dataClient->IsConnected())
    {
        AB::Entities::Service serv;
        serv.uuid = serverId_;
        if (dataClient->Read(serv))
        {
            serv.load = static_cast<uint8_t>(GetAvgLoad());
            serv.heartbeat = sa::time::tick();
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

bool Application::Initialize(const std::vector<std::string>& args)
{
    if (!ServerApp::Initialize(args))
        return false;

    if (!ParseCommandLine())
        return false;

    if (!sa::arg_parser::get_value<bool>(parsedArgs_, "nologo", false))
        ShowLogo();

    GetSubsystem<Asynch::Dispatcher>()->Start();
    GetSubsystem<Asynch::Scheduler>()->Start();

    auto* config = GetSubsystem<IO::SimpleConfigManager>();
    if (configFile_.empty())
    {
#if defined(WIN_SERVICE)
        configFile_ = Utils::ConcatPath(path_, "abfile_svc.lua");
#else
        configFile_ = Utils::ConcatPath(path_, "abfile.lua");
#endif
    }

    if (!config->Load(configFile_))
    {
        LOG_ERROR << "Error loading config file " << configFile_ << std::endl;
        return false;
    }

    if (Utils::Uuid::IsEmpty(serverId_))
        serverId_ = config->GetGlobalString("server_id", Utils::Uuid::EMPTY_UUID);
    if (machine_.empty())
        machine_ = config->GetGlobalString("machine", "");
    if (serverName_.empty())
        serverName_ = config->GetGlobalString("server_name", "abfile");
    if (serverLocation_.empty())
        serverLocation_ = config->GetGlobalString("location", "--");
    if (logDir_.empty())
        logDir_ = config->GetGlobalString("log_dir", "");

    if (serverIp_.empty())
        serverIp_ = config->GetGlobalString("file_ip", "");
    if (serverPort_ == std::numeric_limits<uint16_t>::max())
        serverPort_ = static_cast<uint16_t>(config->GetGlobalInt("file_port", 8081ll));
    else if (serverPort_ == 0)
        serverPort_ = Net::ServiceManager::GetFreePort();

    std::string key = config->GetGlobalString("server_key", "server.key");
    std::string cert = config->GetGlobalString("server_cert", "server.crt");
    size_t threads = static_cast<size_t>(config->GetGlobalInt("num_threads", 0ll));
    if (threads == 0)
        threads = std::max<size_t>(1, std::thread::hardware_concurrency());
    root_ = config->GetGlobalString("root_dir", "");
    dataHost_ = config->GetGlobalString("data_host", "");
    dataPort_ = static_cast<uint16_t>(config->GetGlobalInt("data_port", 0ll));
    requireAuth_ = config->GetGlobalBool("require_auth", false);
    maxThroughput_ = static_cast<uint64_t>(config->GetGlobalInt("max_throughput", 0ll));

    Auth::BanManager::LoginTries = static_cast<uint32_t>(config->GetGlobalInt("login_tries", 5ll));
    Auth::BanManager::LoginRetryTimeout = static_cast<uint32_t>(config->GetGlobalInt("login_retrytimeout", 5000ll));

    if (!logDir_.empty())
    {
        // Different log dir
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    try
    {
        server_ = std::make_unique<HttpsServer>(cert, key);
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR << ex.what() << std::endl;
        LOG_INFO << "If SSL keys are missing, create them by running `openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes -keyout \"" <<
                    key << "\" -out \"" << cert << "\"` in the `bin` directory" << std::endl;
        return false;
    }

    server_->config.port = serverPort_;
    if (!serverIp_.empty())
        server_->config.address = serverIp_;
    server_->config.thread_pool_size = threads;
    server_->io_service = ioService_;

    server_->on_error = std::bind(&Application::HandleError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->default_resource["GET"] = std::bind(&Application::GetHandlerDefault, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->on_accept = std::bind(&Application::HandleOnAccept, shared_from_this(),
        std::placeholders::_1);

    bool haveData = !dataHost_.empty() && (dataPort_ != 0);
    if (!haveData)
    {
        LOG_ERROR << "No data server configured" << std::endl;
        return false;
    }

    server_->resource["^/_version_$"]["GET"] = std::bind(&Application::GetHandlerVersion, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_versions_$"]["GET"] = std::bind(&Application::GetHandlerVersions, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_games_$"]["GET"] = std::bind(&Application::GetHandlerGames, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/(.+)/_files_$"]["GET"] = std::bind(&Application::GetHandlerFiles, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_skills_$"]["GET"] = std::bind(&Application::GetHandlerSkills, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_professions_$"]["GET"] = std::bind(&Application::GetHandlerProfessions, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_attributes_$"]["GET"] = std::bind(&Application::GetHandlerAttributes, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_effects_$"]["GET"] = std::bind(&Application::GetHandlerEffects, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_items_$"]["GET"] = std::bind(&Application::GetHandlerItems, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_quests_$"]["GET"] = std::bind(&Application::GetHandlerQuests, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_music_$"]["GET"] = std::bind(&Application::GetHandlerMusic, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_news_$"]["GET"] = std::bind(&Application::GetHandlerNews, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    auto* dataClient = GetSubsystem<IO::DataClient>();
    LOG_INFO << "Connecting to data server...";
    dataClient->Connect(dataHost_, dataPort_);
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

    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << GetServerId() << std::endl;
    LOG_INFO << "  Name: " << serverName_ << std::endl;
    LOG_INFO << "  Machine: " << machine_ << std::endl;
    LOG_INFO << "  Location: " << serverLocation_ << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << (serverIp_.empty() ? "0.0.0.0" : serverIp_) << ":" << serverPort_ << std::endl;
    LOG_INFO << "  Temporary: " << temporary_ << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Require authentication: " << requireAuth_ << std::endl;
    LOG_INFO << "  Max. throughput: " << Utils::ConvertSize(maxThroughput_) << "/s" << std::endl;
    LOG_INFO << "  Worker Threads: " << server_->config.thread_pool_size << std::endl;
    if (haveData)
        LOG_INFO << "  Data Server: " << dataClient->GetHost() << ":" << dataClient->GetPort() << std::endl;
    else
        LOG_INFO << "  Data Server: (NONE)" << std::endl;
    LOG_INFO << "  Message Server: " << msgClient->GetHost() << ":" << msgClient->GetPort() << std::endl;

    return true;
}

void Application::Run()
{
    startTime_ = sa::time::tick();
    statusMeasureTime_ = startTime_;
    uptimeRound_ = 1;
    AB::Entities::Service serv;
    serv.uuid = serverId_;
    auto* dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(serv))
    {
        if (!temporary_)
        {
            // Temporary services do not exist in DB
            LOG_WARNING << "Unable to read service with UUID " << serv.uuid << std::endl;
        }
    }

    UpdateService(serv);
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.startTime = startTime_;
    serv.temporary = temporary_;
    serv.heartbeat = startTime_;
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
    server_->start();
    ioService_->run();
}

void Application::Stop()
{
    if (!running_)
        return;

    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;

    AB::Entities::Service serv;
    serv.uuid = serverId_;

    auto* dataClient = GetSubsystem<IO::DataClient>();

    if (dataClient->Read(serv))
    {
        serv.status = AB::Entities::ServiceStatusOffline;
        serv.stopTime = sa::time::tick();
        if (serv.startTime != 0)
            serv.runTime += (serv.stopTime - serv.startTime) / 1000;

        SendServerLeft(GetSubsystem<Net::MessageClient>(), serv);

        if (!temporary_)
            dataClient->Update(serv);
        else
            // If autoterm -> temporary -> dynamically spawned -> delete from DB
            dataClient->Delete(serv);

        AB::Entities::ServiceList sl;
        dataClient->Invalidate(sl);
    }
    else
        LOG_ERROR << "Unable to read service" << std::endl;

    server_->stop();
    ioService_->stop();
}

bool Application::IsAllowed(std::shared_ptr<HttpsServer::Request> request)
{
    uint32_t ip = request->remote_endpoint->address().to_v4().to_uint();
    auto* banMan = GetSubsystem<Auth::BanManager>();
    if (banMan->IsIpBanned(ip))
    {
        LOG_WARNING << "IP " << Utils::ConvertIPToString(ip) << " is banned" << std::endl;
        return false;
    }

    if (!requireAuth_)
        return true;

    auto* dataClient = GetSubsystem<IO::DataClient>();

    // Check Auth
    const auto it = request->header.find("Auth");
    if (it == request->header.end())
    {
        LOG_WARNING << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Missing Auth header" << std::endl;
        banMan->AddLoginAttempt(ip, false);
        return false;
    }
    const std::string accId = (*it).second.substr(0, 36);
    const std::string token = (*it).second.substr(36);
    if (Utils::Uuid::IsEmpty(token) || Utils::Uuid::IsEmpty(accId))
    {
        LOG_WARNING << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Wrong Auth header " << (*it).second << std::endl;
        banMan->AddLoginAttempt(ip, false);
        return false;
    }

    AB::Entities::Account acc;
    acc.uuid = accId;
    if (!dataClient->Read(acc))
    {
        LOG_WARNING << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Unable to read account " << accId << std::endl;
        return false;
    }
    if (acc.status != AB::Entities::AccountStatusActivated)
        return false;
    if (banMan->IsAccountBanned(uuids::uuid(acc.uuid)))
    {
        LOG_WARNING << "Account " << acc.uuid << " is banned" << std::endl;
        banMan->AddLoginAttempt(ip, false);
        return false;
    }

    if (!Utils::Uuid::IsEqual(acc.authToken, token))
    {
        banMan->AddLoginAttempt(ip, false);
        return false;
    }
    if (sa::time::is_expired(acc.authTokenExpiry))
    {
        // Expired auth token
        banMan->AddLoginAttempt(ip, false);
        return false;
    }
    banMan->AddLoginAttempt(ip, true);

    return true;
}

SimpleWeb::CaseInsensitiveMultimap Application::GetDefaultHeader()
{
    SimpleWeb::CaseInsensitiveMultimap result;
    result.emplace("Accept-Ranges", "bytes");
    result.emplace("Server", "abfile");
    return result;
}

void Application::SendFileRange(std::shared_ptr<HttpsServer::Response> response,
    const std::string& path,
    const sa::http::range& range,
    bool multipart, const std::string& boundary)
{
    auto ifs = std::make_shared<std::ifstream>();
    ifs->open(path, std::ifstream::in | std::ios::binary);
    ASSERT(ifs);

    ifs->seekg(0, std::ios::end);
    auto fileSize = (long)ifs->tellg();
    size_t start = range.start;
    size_t end = (range.end != 0) ? range.end : (size_t)fileSize;
    ASSERT(end > start);
    size_t length = end - start;

    ifs->seekg(start, std::ios::beg);
    UpdateBytesSent(static_cast<size_t>(length));

    if (multipart)
    {
        response->write("--" + boundary);
        response->write("Content-Type: application/octet-stream\n");
        response->write("Content-Range: " + std::to_string(range.start) + "-" +
            std::to_string(range.end) + "/" + std::to_string(fileSize) + "\n");
        response->write("\n");
    }

    struct FileServer
    {
        static void ReadAndSend(uint64_t maxBytePerMSec, const std::shared_ptr<HttpsServer::Response>& response,
            const std::shared_ptr<std::ifstream>& ifs, size_t remaining)
        {
            size_t chunkSize = std::min<size_t>(131072u, remaining);
            std::vector<char> buffer;
            buffer.resize(chunkSize);
            std::streamsize read_length;
            sa::time::timer timer;
            if ((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0)
            {
                response->write(&buffer[0], read_length);
                if (read_length == static_cast<std::streamsize>(buffer.size()))
                {
                    response->send([maxBytePerMSec, response, ifs, remaining, read_length](const SimpleWeb::error_code& ec)
                    {
                        if (!ec)
                        {
                            if ((long)remaining > read_length)
                                ReadAndSend(maxBytePerMSec, response, ifs, remaining - read_length);
                        }
                        else
                            LOG_ERROR << "Connection interrupted " << ec.default_error_condition().value() << " " <<
                            ec.default_error_condition().message() << std::endl;
                    });
                }

                if (maxBytePerMSec > 0)
                {
                    int64_t time = timer.elapsed_millis();
                    if (time == 0)
                        time = 1;
                    uint64_t bytePerMs = (uint64_t)read_length / (uint64_t)time;
                    if (maxBytePerMSec < bytePerMs)
                    {
                        uint64_t diff = bytePerMs - maxBytePerMSec;
                        auto sleepMs = std::clamp<unsigned>(static_cast<unsigned>(((float)diff / (float)(1000 - time))) / 12,
                            1u, 100u);
                        // Throttle to meet max throughput
                        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
                    }
                }
            }
        }
    };
    FileServer::ReadAndSend(maxThroughput_ / 1000, response, ifs, length);
}

void Application::GetHandlerDefault(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }
    try
    {
        auto web_root_path = fs::canonical(root_);
        auto path = fs::canonical(root_ + request->path);
        // Check if path is within web_root_path
        if (std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
            !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
        {
            LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
                << "Trying to access file outside root " << path.string() << std::endl;
            throw std::invalid_argument("path must be within root path");
        }
        if (fs::is_directory(path))
        {
            LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
                << "Trying to access a directory " << path.string() << std::endl;
            throw std::invalid_argument("not a file");
        }
        if (Utils::IsHiddenFile(path.string()))
        {
            LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
                << "Trying to access a hidden file " << path.string() << std::endl;
            throw std::invalid_argument("hidden file");
        }

        std::ifstream ifs(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);
        if (!ifs)
            throw std::invalid_argument("could not read file");

        auto fileSize = ifs.tellg();

        const auto rangeHeaderIt = request->header.find("Range");
        sa::http::ranges ranges;
        if (rangeHeaderIt == request->header.end())
        {
            ranges.push_back({ 0, 0, 0 });
        }
        else
        {
            if (!sa::http::parse_ranges(fileSize, rangeHeaderIt->second, ranges))
            {
                response->write(SimpleWeb::StatusCode::client_error_range_not_satisfiable,
                    "Range Not Satisfiable");
                return;
            }
        }

        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Range_requests
        const bool multipart = ranges.size() > 1;
        const bool isRange = !sa::http::is_full_range(fileSize, ranges[0]);
        // Multipart not supported
        if (multipart)
        {
            LOG_WARNING << "TODO: Multipart not supported" << std::endl;
            response->write(SimpleWeb::StatusCode::client_error_range_not_satisfiable,
                "Range Not Satisfiable");
            return;
        }

        const std::string boundary = "3d6b6a416f9b5";

        SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();

        if (isRange && !multipart)
        {
            // Single part of a file
            header.emplace("Content-Type", "application/octet-stream");
            header.emplace("Content-Length", std::to_string(ranges[0].length));
            header.emplace("Content-Range", std::to_string(ranges[0].start) + "-" +
                std::to_string(ranges[0].end) + "/" + std::to_string(fileSize));
            response->write(SimpleWeb::StatusCode::success_partial_content, header);
        }
        else if (isRange && multipart)
        {
            // Multiple parts of a file in one response -> multipart message
            header.emplace("Content-Type", "multipart/byteranges; boundary=" + boundary);
            header.emplace("Content-Length", std::to_string(sa::http::content_length(ranges)));
            response->write(SimpleWeb::StatusCode::success_partial_content, header);
        }
        else
        {
            // Whole file
            header.emplace("Content-Type", "application/octet-stream");
            header.emplace("Content-Length", std::to_string(fileSize));
            response->write(header);
        }

        SendFileRange(response, path.string(), ranges[0], multipart, boundary);

#if 0
        if (multipart)
            response->write("--" + boundary + "--\n");
#endif
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR << "Exception " << ex.what() << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request->path);
    }
}

void Application::GetHandlerFiles(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }
    std::string platform = Utils::ExtractFileDir(request->path);
    auto web_root_path = fs::canonical(root_);
    if (!fs::is_directory(web_root_path))
    {
        LOG_ERROR << "Directory not found " << web_root_path.string() << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "No Found");
        return;
    }

    // Return an index of files with checksum
    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("files");

    auto addFile = [&](const fs::directory_entry& p, bool common)
    {
        if (p.is_directory())
            return;
        if (p.path().extension() == ".meta" || p.path().extension() == ".sha1")
            return;

        const std::string fullpath = p.path().string();
        const std::string filename = p.path().filename().string();
        if (Utils::IsHiddenFile(fullpath))
            return;

        // There must be a correcponding meta file
        if (!Utils::FileExists(fullpath + ".meta"))
            return;
        if (!Utils::FileExists(fullpath + ".sha1"))
            return;

        std::ifstream shafile(fullpath + ".sha1", std::ios::in);
        shafile.seekg(0, std::ios::beg);
        std::stringstream shastream;
        if (shafile)
            shastream << shafile.rdbuf();

        std::string realitvename = fullpath.substr(web_root_path.string().size() + 1);
        sa::ReplaceSubstring<char>(realitvename, "\\", "/");
        std::string basepath = !common ?
            realitvename.substr(platform.length()) :
            realitvename;

        auto gNd = root.append_child("file");
        gNd.append_attribute("path").set_value(realitvename.c_str());
        gNd.append_attribute("base_path").set_value(basepath.c_str());
        gNd.append_attribute("sha1").set_value(shastream.str().c_str());
    };

    // Common files are in file_root
    for (const auto& p : fs::directory_iterator(web_root_path))
        addFile(p, true);

    // Platform specific files in file_root/(platform)
    if (!platform.empty())
    {
        try
        {
            fs::path platform_path = web_root_path;
            platform_path += platform;
            if (fs::is_directory(platform_path))
            {
                for (const auto& p : fs::directory_iterator(platform_path))
                    addFile(p, false);
            }
            else
                LOG_WARNING << platform_path.string() << " does not exist, but was requested" << std::endl;
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR << ex.what() << std::endl;
        }
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerGames(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::GameList gl;
    if (!dataClient->Read(gl))
    {
        LOG_ERROR << "Error reading game list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    AB::Entities::Version gamesVersion;
    gamesVersion.name = "game_maps";
    if (!dataClient->Read(gamesVersion))
    {
        LOG_ERROR << "Error reading game version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("games");
    root.append_attribute("version").set_value(gamesVersion.value);

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (!dataClient->Read(g))
            continue;
        auto gNd = root.append_child("game");
        gNd.append_attribute("uuid").set_value(g.uuid.c_str());
        gNd.append_attribute("name").set_value(g.name.c_str());
        gNd.append_attribute("type").set_value(g.type);
        gNd.append_attribute("landing").set_value(g.landing);
        gNd.append_attribute("map_coord_x").set_value(g.mapCoordX);
        gNd.append_attribute("map_coord_y").set_value(g.mapCoordY);
        // The client should know about that to show/hide the 'Enter' button
        gNd.append_attribute("queue_map").set_value(g.queueMapUuid.c_str());
        // The rest is not interesting for the player, so skip it
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerSkills(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::SkillList sl;
    if (!dataClient->Read(sl))
    {
        LOG_ERROR << "Error reading skill list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_skills";
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading skill version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("skills");
    root.append_attribute("version").set_value(v.value);

    for (const std::string& uuid : sl.skillUuids)
    {
        AB::Entities::Skill s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto gNd = root.append_child("skill");
        gNd.append_attribute("uuid").set_value(s.uuid.c_str());
        gNd.append_attribute("index").set_value(s.index);
        gNd.append_attribute("name").set_value(s.name.c_str());
        gNd.append_attribute("attribute").set_value(s.attributeUuid.c_str());
        gNd.append_attribute("profession").set_value(s.professionUuid.c_str());
        gNd.append_attribute("type").set_value(static_cast<unsigned long long>(s.type));
        gNd.append_attribute("elite").set_value(s.isElite);
        gNd.append_attribute("access").set_value(s.access);
        gNd.append_attribute("description").set_value(s.description.c_str());
        gNd.append_attribute("short_description").set_value(s.shortDescription.c_str());
        gNd.append_attribute("icon").set_value(s.icon.c_str());
        gNd.append_attribute("sound_effect").set_value(s.soundEffect.c_str());
        gNd.append_attribute("particle_effect").set_value(s.particleEffect.c_str());
        gNd.append_attribute("activation").set_value(s.activation);
        gNd.append_attribute("recharge").set_value(s.recharge);
        gNd.append_attribute("const_energy").set_value(s.costEnergy);
        gNd.append_attribute("const_energy_regen").set_value(s.costEnergyRegen);
        gNd.append_attribute("const_adrenaline").set_value(s.costAdrenaline);
        gNd.append_attribute("const_overcast").set_value(s.costOvercast);
        gNd.append_attribute("const_hp").set_value(s.costHp);
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerProfessions(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::ProfessionList pl;
    if (!dataClient->Read(pl))
    {
        LOG_ERROR << "Error reading profession list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_professions";
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading profession version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("professions");
    root.append_attribute("version").set_value(v.value);

    for (const std::string& uuid : pl.profUuids)
    {
        AB::Entities::Profession s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto gNd = root.append_child("prof");
        gNd.append_attribute("uuid").set_value(s.uuid.c_str());
        gNd.append_attribute("index").set_value(s.index);
        gNd.append_attribute("name").set_value(s.name.c_str());
        gNd.append_attribute("abbr").set_value(s.abbr.c_str());
        gNd.append_attribute("model_index_female").set_value(s.modelIndexFemale);
        gNd.append_attribute("model_index_male").set_value(s.modelIndexMale);
        gNd.append_attribute("num_attr").set_value(s.attributeCount);
        for (const AB::Entities::AttriInfo& a : s.attributes)
        {
            auto attrNd = gNd.append_child("attr");
            attrNd.append_attribute("uuid").set_value(a.uuid.c_str());
        }
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerAttributes(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AttributeList pl;
    if (!dataClient->Read(pl))
    {
        LOG_ERROR << "Error reading attribute list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_attributes";
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading attribute version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("attributes");
    root.append_attribute("version").set_value(v.value);

    for (const std::string& uuid : pl.uuids)
    {
        AB::Entities::Attribute s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto gNd = root.append_child("attrib");
        gNd.append_attribute("uuid").set_value(s.uuid.c_str());
        gNd.append_attribute("index").set_value(s.index);
        gNd.append_attribute("name").set_value(s.name.c_str());
        gNd.append_attribute("profession").set_value(s.professionUuid.c_str());
        gNd.append_attribute("primary").set_value(s.isPrimary);
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerEffects(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::EffectList pl;
    if (!dataClient->Read(pl))
    {
        LOG_ERROR << "Error reading effect list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_effects";
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading effect version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("effects");
    root.append_attribute("version").set_value(v.value);

    for (const std::string& uuid : pl.effectUuids)
    {
        AB::Entities::Effect s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto gNd = root.append_child("effect");
        gNd.append_attribute("uuid").set_value(s.uuid.c_str());
        gNd.append_attribute("index").set_value(s.index);
        gNd.append_attribute("name").set_value(s.name.c_str());
        gNd.append_attribute("category").set_value(s.category);
        gNd.append_attribute("icon").set_value(s.icon.c_str());
        gNd.append_attribute("sound_effect").set_value(s.soundEffect.c_str());
        gNd.append_attribute("particle_effect").set_value(s.particleEffect.c_str());
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerItems(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::ItemList pl;
    if (!dataClient->Read(pl))
    {
        LOG_ERROR << "Error reading item list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_items";
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading items version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("items");
    root.append_attribute("version").set_value(v.value);

    for (const std::string& uuid : pl.itemUuids)
    {
        AB::Entities::Item s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto gNd = root.append_child("item");
        gNd.append_attribute("uuid").set_value(s.uuid.c_str());
        gNd.append_attribute("index").set_value(s.index);
        gNd.append_attribute("model_class").set_value(static_cast<uint32_t>(s.model_class));
        gNd.append_attribute("name").set_value(s.name.c_str());
        gNd.append_attribute("type").set_value(static_cast<int>(s.type));
        gNd.append_attribute("object").set_value(s.objectFile.c_str());
        gNd.append_attribute("icon").set_value(s.iconFile.c_str());
        gNd.append_attribute("item_flags").set_value(s.itemFlags);
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerQuests(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::QuestList gl;
    if (!dataClient->Read(gl))
    {
        LOG_ERROR << "Error reading game list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    AB::Entities::Version gamesVersion;
    gamesVersion.name = "game_quests";
    if (!dataClient->Read(gamesVersion))
    {
        LOG_ERROR << "Error reading game version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("quests");
    root.append_attribute("version").set_value(gamesVersion.value);

    for (const std::string& uuid : gl.questUuids)
    {
        AB::Entities::Quest g;
        g.uuid = uuid;
        if (!dataClient->Read(g))
            continue;
        auto gNd = root.append_child("game");
        gNd.append_attribute("uuid").set_value(g.uuid.c_str());
        gNd.append_attribute("index").set_value(g.index);
        gNd.append_attribute("name").set_value(g.name.c_str());
        gNd.append_attribute("description").set_value(g.description.c_str());
        gNd.append_attribute("reward_xp").set_value(g.rewardXp);
        gNd.append_attribute("reward_money").set_value(g.rewardMoney);
        gNd.append_attribute("reward_items").set_value(sa::CombineString(g.rewardItems, std::string(";")).c_str());
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerMusic(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::MusicList pl;
    if (!dataClient->Read(pl))
    {
        LOG_ERROR << "Error reading music list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_music";
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading music version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("music_list");
    root.append_attribute("version").set_value(v.value);

    for (const std::string& uuid : pl.musicUuids)
    {
        AB::Entities::Music s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto gNd = root.append_child("music");
        gNd.append_attribute("uuid").set_value(s.uuid.c_str());
        gNd.append_attribute("map_uuid").set_value(s.mapUuid.c_str());
        gNd.append_attribute("local_file").set_value(s.localFile.c_str());
        gNd.append_attribute("remote_file").set_value(s.remoteFile.c_str());
        gNd.append_attribute("sorting").set_value(s.sorting);
        gNd.append_attribute("style").set_value(static_cast<uint32_t>(s.style));
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerVersion(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    std::string table;
    auto query_fields = request->parse_query_string();
    for (const auto& field : query_fields)
    {
        if (field.first.compare("entity") == 0)
        {
            table = field.second;
            break;
        }
    }
    if (table.empty())
    {
        LOG_ERROR << "Empty table" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Version v;
    v.name = table;
    if (!dataClient->Read(v))
    {
        LOG_ERROR << "Error reading version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    if (v.isInternal)
    {
        LOG_ERROR << "Error reading internal version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    std::stringstream stream;
    stream << v.value;
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/plain");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerVersions(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::VersionList vl;
    if (!dataClient->Read(vl))
    {
        LOG_ERROR << "Error reading version list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("versions");

    for (const AB::Entities::Version& v : vl.versions)
    {
        auto gNd = root.append_child("version");
        gNd.append_attribute("uuid").set_value(v.uuid.c_str());
        gNd.append_attribute("name").set_value(v.name.c_str());
        gNd.append_attribute("value").set_value(v.value);
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::GetHandlerNews(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    if (!IsAllowed(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::LatestNewsList vl;
    if (!dataClient->Read(vl))
    {
        LOG_ERROR << "Error reading news list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("news_list");

    for (const std::string& uuid : vl.uuids)
    {
        AB::Entities::News n;
        n.uuid = uuid;
        if (!dataClient->Read(n))
            continue;

        auto gNd = root.append_child("news");
        gNd.append_attribute("created").set_value(n.created);
        gNd.append_attribute("body").set_value(Utils::XML::Escape(n.body).c_str());
    }

    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::HandleError(std::shared_ptr<HttpsServer::Request> request, const SimpleWeb::error_code& ec)
{
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    if (ec.default_error_condition().value() == 995 || ec == SimpleWeb::errc::operation_canceled)
        return;

    LOG_ERROR << "(" << ec.default_error_condition().value() << ") " << ec.default_error_condition().message() <<
        " from " << Utils::ConvertIPToString(request->remote_endpoint->address().to_v4().to_uint()) << std::endl;
}

bool Application::HandleOnAccept(const asio::ip::tcp::endpoint& endpoint)
{
    uint32_t ip = endpoint.address().to_v4().to_uint();
    auto* banMan = GetSubsystem<Auth::BanManager>();
    if (!banMan->AcceptConnection(ip))
    {
        LOG_WARNING << "Connection attempt from disabled IP " << Utils::ConvertIPToString(ip) << std::endl;
        return false;
    }
    if (banMan->IsIpBanned(ip))
    {
        LOG_WARNING << "Connection attempt from banned IP " << Utils::ConvertIPToString(ip) << std::endl;
        return false;
    }
    return true;
}
