#include "stdafx.h"
#include "Application.h"
#include "SimpleConfigManager.h"
#include <sstream>
#include <fstream>
#include "Logger.h"
#include "DataClient.h"
#include <AB/Entities/GameList.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/Skill.h>
#include <AB/Entities/SkillList.h>
#include <AB/Entities/ProfessionList.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Version.h>
#include <AB/Entities/Attribute.h>
#include <AB/Entities/AttributeList.h>
#include <AB/Entities/IpBan.h>
#include <AB/Entities/Ban.h>
#include <AB/Entities/EffectList.h>
#include <AB/Entities/Effect.h>
#include <AB/Entities/AccountBan.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/ServiceList.h>
#include <pugixml.hpp>
#include "Profiler.h"
#include "Utils.h"
#include <abcrypto.hpp>
#include "StringUtils.h"

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    requireAuth_(false),
    ioService_(),
    startTime_(0),
    bytesSent_(0),
    uptimeRound_(0),
    statusMeasureTime_(0),
    lastLoadCalc_(0)
{
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
        else if (a.compare("-h") == 0 || a.compare("-help") == 0)
        {
            return false;
        }
    }
    return true;
}

void Application::ShowHelp()
{
    std::cout << "abserv [-<options> [<value>]]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "  conf <config file>: Use config file" << std::endl;
    std::cout << "  log <log directory>: Use log directory" << std::endl;
    std::cout << "  h, help: Show help" << std::endl;
}

void Application::UpdateBytesSent(size_t bytes)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (bytesSent_ + bytes > std::numeric_limits<uint64_t>::max())
    {
        bytesSent_ = 0;
        statusMeasureTime_ = Utils::AbTick();
        ++uptimeRound_;
    }
    bytesSent_ += bytes;

    // Calculate load
    if ((Utils::AbTick() - lastLoadCalc_) > 1000 || loads_.empty())
    {
        lastLoadCalc_ = Utils::AbTick();

        uint8_t load = 0;
        if (maxThroughput_ != 0)
        {
            int64_t mesTime = Utils::AbTick() - statusMeasureTime_;
            int bytesPerSecond = static_cast<int>(bytesSent_ / (mesTime / 1000));
            float ld = ((float)bytesPerSecond / (float)maxThroughput_) * 100.0f;
            load = static_cast<uint8_t>(ld);
            if (load > 100)
                load = 100;
        }

        while (loads_.size() > 9)
            loads_.erase(loads_.begin());
        loads_.push_back(static_cast<int>(load));

        AB::Entities::Service serv;
        serv.uuid = serverId_;
        if (dataClient_->Read(serv))
        {
            uint8_t avgLoad = GetAvgLoad();
            if (avgLoad != serv.load)
            {
                serv.load = avgLoad;
                dataClient_->Update(serv);
            }
        }
    }
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

    if (configFile_.empty())
        configFile_ = path_ + "/abfile.lua";
    if (!IO::SimpleConfigManager::Instance.Load(configFile_))
    {
        LOG_ERROR << "Error loading config file " << configFile_ << std::endl;
        return false;
    }

    serverId_ = IO::SimpleConfigManager::Instance.GetGlobal("server_id", "00000000-0000-0000-0000-000000000000");
    std::string address = IO::SimpleConfigManager::Instance.GetGlobal("server_ip", "");
    uint16_t port = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("server_port", 8081));
    std::string key = IO::SimpleConfigManager::Instance.GetGlobal("server_key", "server.key");
    std::string cert = IO::SimpleConfigManager::Instance.GetGlobal("server_cert", "server.crt");
    size_t threads = IO::SimpleConfigManager::Instance.GetGlobal("num_threads", 1);
    root_ = IO::SimpleConfigManager::Instance.GetGlobal("root_dir", "");
    logDir_ = IO::SimpleConfigManager::Instance.GetGlobal("log_dir", "");
    dataHost_ = IO::SimpleConfigManager::Instance.GetGlobal("data_host", "");
    dataPort_ = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("data_port", 0));
    requireAuth_ = IO::SimpleConfigManager::Instance.GetGlobalBool("require_auth", false);
    adminPassword_ = IO::SimpleConfigManager::Instance.GetGlobal("abfile_admin_pass", "");
    maxThroughput_ = IO::SimpleConfigManager::Instance.GetGlobal("max_throughput", 0);

    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    server_ = std::make_unique<HttpsServer>(cert, key);
    server_->config.port = port;
    if (!address.empty())
        server_->config.address = address;
    server_->config.thread_pool_size = threads;

    server_->default_resource["GET"] = std::bind(&Application::GetHandlerDefault, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    bool haveData = !dataHost_.empty() && (dataPort_ != 0);

    if (haveData)
    {
        server_->resource["^/_version_$"]["GET"] = std::bind(&Application::GetHandlerVersion, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2);
        server_->resource["^/_games_$"]["GET"] = std::bind(&Application::GetHandlerGames, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2);
        server_->resource["^/_skills_$"]["GET"] = std::bind(&Application::GetHandlerSkills, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2);
        server_->resource["^/_professions_$"]["GET"] = std::bind(&Application::GetHandlerProfessions, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2);
        server_->resource["^/_attributes_$"]["GET"] = std::bind(&Application::GetHandlerAttributes, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2);
        server_->resource["^/_effects_$"]["GET"] = std::bind(&Application::GetHandlerEffects, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2);
    }
    server_->resource["^/_status_$"]["GET"] = std::bind(&Application::GetHandlerStatus, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    server_->on_error = std::bind(&Application::HandleError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    if (haveData)
    {
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
    }
    else
    {
        LOG_WARNING << "Not connected to data server" << std::endl;
    }

    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Server ID: " << serverId_ << std::endl;
    LOG_INFO << "  Location: " << IO::SimpleConfigManager::Instance.GetGlobal("location", "--") << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << (address.empty() ? "0.0.0.0" : address) << ":" << port << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Require authentication: " << (requireAuth_ ? "true" : "false") << std::endl;
    LOG_INFO << "  Max. throughput: " << Utils::ConvertSize(maxThroughput_) << "/s" << std::endl;
    if (haveData)
        LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;
    else
        LOG_INFO << "  Data Server: (NONE)" << std::endl;

    return true;
}

void Application::Run()
{
    startTime_ = Utils::AbTick();
    statusMeasureTime_ = startTime_;
    uptimeRound_ = 1;
    AB::Entities::Service serv;
    serv.uuid = serverId_;
    dataClient_->Read(serv);
    serv.name = "abfile";
    serv.location = IO::SimpleConfigManager::Instance.GetGlobal("location", "--");
    serv.host = IO::SimpleConfigManager::Instance.GetGlobal("server_host", "");
    serv.port = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("server_port", 8081));
    serv.file = exeFile_;
    serv.path = path_;
    serv.arguments = Utils::CombineString(arguments_, std::string(" "));
    serv.status = AB::Entities::ServiceStatusOnline;
    serv.type = AB::Entities::ServiceTypeFileServer;
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
    dataClient_->Read(serv);
    serv.status = AB::Entities::ServiceStatusOffline;
    serv.stopTime = Utils::AbTick();
    if (serv.startTime != 0)
        serv.runTime += (serv.stopTime - serv.startTime) / 1000;
    dataClient_->UpdateOrCreate(serv);

    AB::Entities::ServiceList sl;
    dataClient_->Invalidate(sl);

    server_->stop();
}

bool Application::IsAllowed(std::shared_ptr<HttpsServer::Request> request)
{
    uint32_t ip = request->remote_endpoint->address().to_v4().to_uint();
    AB::Entities::IpBan ban;
    ban.ip = ip;
    ban.mask = 0xFFFFFFFF;
    if (dataClient_->Read(ban))
    {
        AB::Entities::Ban _ban;
        _ban.uuid = ban.banUuid;
        if (dataClient_->Read(_ban))
        {
            if (_ban.active && (_ban.expires <= 0) || (_ban.expires >= Utils::AbTick() / 1000))
                return false;
        }
    }

    if (!requireAuth_)
        return true;

    // Check Auth
    const auto it = request->header.find("Auth");
    if (it == request->header.end())
    {
        LOG_WARNING << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Missing Auth header" << std::endl;
        return false;
    }
    const std::string accId = (*it).second.substr(0, 36);
    const std::string passwd = (*it).second.substr(36);
    if (accId.empty() || passwd.empty())
    {
        LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Wrong Auth header " << (*it).second << std::endl;
        return false;
    }

    AB::Entities::Account acc;
    acc.uuid = accId;
    if (!dataClient_->Read(acc))
    {
        LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Unable to read account " << accId << std::endl;
        return false;
    }
    if (acc.status != AB::Entities::AccountStatusActivated)
        return false;
    if (IsAccountBanned(acc))
        return false;

    if (bcrypt_checkpass(passwd.c_str(), acc.password.c_str()) != 0)
        return false;

    return true;
}

bool Application::IsAdmin(std::shared_ptr<HttpsServer::Request> request)
{
    if (adminPassword_.empty())
        return true;

    // Check Auth
    const auto it = request->header.find("AuthAdmin");
    if (it == request->header.end())
    {
        LOG_WARNING << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Missing AuthAdmin header" << std::endl;
        return false;
    }
    const std::string& passwd = (*it).second;
    if (passwd.empty())
    {
        LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Wrong AuthAdmin header " << (*it).second << std::endl;
        return false;
    }

    return passwd.compare(adminPassword_) == 0;
}

bool Application::IsAccountBanned(const AB::Entities::Account& acc)
{
    AB::Entities::AccountBan ban;
    ban.accountUuid = acc.uuid;
    if (!dataClient_->Read(ban))
        return false;
    AB::Entities::Ban _ban;
    _ban.uuid = ban.banUuid;
    if (!dataClient_->Read(_ban))
        return false;
    if (!_ban.active)
        return false;
    return (_ban.expires <= 0) || (_ban.expires >= Utils::AbTick() / 1000);
}

bool Application::IsHiddenFile(const boost::filesystem::path& path)
{
    auto name = path.filename();
    if (name != ".." &&
        name != "."  &&
        (name.string()[0] == '.') || (name.string().find("/.") != std::string::npos))
        return true;

    return false;
}

SimpleWeb::CaseInsensitiveMultimap Application::GetDefaultHeader()
{
    SimpleWeb::CaseInsensitiveMultimap result;
    result.emplace("Server", "abfile");
    return result;
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
        auto web_root_path = boost::filesystem::canonical(root_);
        auto path = boost::filesystem::canonical(web_root_path / request->path);
        // Check if path is within web_root_path
        if (std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
            !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
        {
            LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
                << "Trying to access file outside root " << path.string() << std::endl;
            throw std::invalid_argument("path must be within root path");
        }
        if (boost::filesystem::is_directory(path))
        {
            LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
                << "Trying to access a directory " << path.string() << std::endl;
            throw std::invalid_argument("not a file");
        }
        if (IsHiddenFile(path))
        {
            LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
                << "Trying to access a hidden file " << path.string() << std::endl;
            throw std::invalid_argument("hidden file");
        }

        SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();

        auto ifs = std::make_shared<std::ifstream>();
        ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

        if (*ifs)
        {
            auto length = ifs->tellg();
            ifs->seekg(0, std::ios::beg);
            UpdateBytesSent(length);

            header.emplace("Content-Length", to_string(length));
            response->write(header);

            // Trick to define a recursive function within this scope (for example purposes)
            class FileServer
            {
            public:
                static void read_and_send(const std::shared_ptr<HttpsServer::Response> &response,
                    const std::shared_ptr<std::ifstream> &ifs)
                {
                    // Read and send 128 KB at a time
                    std::vector<char> buffer(131072);
                    std::streamsize read_length;
                    if ((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0)
                    {
                        response->write(&buffer[0], read_length);
                        if (read_length == static_cast<std::streamsize>(buffer.size()))
                        {
                            response->send([response, ifs](const SimpleWeb::error_code &ec)
                            {
                                if (!ec)
                                    read_and_send(response, ifs);
                                else
                                    LOG_ERROR << "Connection interrupted" << std::endl;
                            });
                        }
                    }
                }
            };
            FileServer::read_and_send(response, ifs);
        }
        else
            throw std::invalid_argument("could not read file");
    }
    catch (const std::exception&)
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request->path);
    }
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

    AB::Entities::GameList gl;
    if (!dataClient_->Read(gl))
    {
        LOG_ERROR << "Error reading game list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    AB::Entities::Version gamesVersion;
    gamesVersion.name = "game_maps";
    if (!dataClient_->Read(gamesVersion))
    {
        LOG_ERROR << "Error reading game version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";
    declarationNode.append_attribute("standalone") = "yes";
    auto root = doc.append_child("games");
    root.append_attribute("version") = gamesVersion.value;

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (!dataClient_->Read(g))
            continue;
        auto gNd = root.append_child("game");
        gNd.append_attribute("uuid") = g.uuid.c_str();
        gNd.append_attribute("name") = g.name.c_str();
        gNd.append_attribute("type") = g.type;
        gNd.append_attribute("landing") = g.landing;
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

    AB::Entities::SkillList sl;
    if (!dataClient_->Read(sl))
    {
        LOG_ERROR << "Error reading skill list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_skills";
    if (!dataClient_->Read(v))
    {
        LOG_ERROR << "Error reading skill version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";
    declarationNode.append_attribute("standalone") = "yes";
    auto root = doc.append_child("skills");
    root.append_attribute("version") = v.value;

    for (const std::string& uuid : sl.skillUuids)
    {
        AB::Entities::Skill s;
        s.uuid = uuid;
        if (!dataClient_->Read(s))
            continue;
        auto gNd = root.append_child("skill");
        gNd.append_attribute("uuid") = s.uuid.c_str();
        gNd.append_attribute("index") = s.index;
        gNd.append_attribute("name") = s.name.c_str();
        gNd.append_attribute("attribute") = s.attributeUuid.c_str();
        gNd.append_attribute("type") = s.type;
        gNd.append_attribute("elite") = s.isElite;
        gNd.append_attribute("description") = s.description.c_str();
        gNd.append_attribute("short_description") = s.shortDescription.c_str();
        gNd.append_attribute("icon") = s.icon.c_str();
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

    AB::Entities::ProfessionList pl;
    if (!dataClient_->Read(pl))
    {
        LOG_ERROR << "Error reading profession list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_professions";
    if (!dataClient_->Read(v))
    {
        LOG_ERROR << "Error reading profession version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";
    declarationNode.append_attribute("standalone") = "yes";
    auto root = doc.append_child("professions");
    root.append_attribute("version") = v.value;

    for (const std::string& uuid : pl.profUuids)
    {
        AB::Entities::Profession s;
        s.uuid = uuid;
        if (!dataClient_->Read(s))
            continue;
        auto gNd = root.append_child("prof");
        gNd.append_attribute("uuid") = s.uuid.c_str();
        gNd.append_attribute("index") = s.index;
        gNd.append_attribute("name") = s.name.c_str();
        gNd.append_attribute("abbr") = s.abbr.c_str();
        gNd.append_attribute("num_attr") = s.attributeCount;
        for (const std::string& a : s.attributeUuids)
        {
            auto attrNd = gNd.append_child("attr");
            attrNd.append_attribute("uuid") = a.c_str();
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

    AB::Entities::AttributeList pl;
    if (!dataClient_->Read(pl))
    {
        LOG_ERROR << "Error reading attribute list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_attributes";
    if (!dataClient_->Read(v))
    {
        LOG_ERROR << "Error reading attribute version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";
    declarationNode.append_attribute("standalone") = "yes";
    auto root = doc.append_child("attributes");
    root.append_attribute("version") = v.value;

    for (const std::string& uuid : pl.uuids)
    {
        AB::Entities::Attribute s;
        s.uuid = uuid;
        if (!dataClient_->Read(s))
            continue;
        auto gNd = root.append_child("attrib");
        gNd.append_attribute("uuid") = s.uuid.c_str();
        gNd.append_attribute("index") = s.index;
        gNd.append_attribute("name") = s.name.c_str();
        gNd.append_attribute("profession") = s.professionUuid.c_str();
        gNd.append_attribute("primary") = s.isPrimary;
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

    AB::Entities::EffectList pl;
    if (!dataClient_->Read(pl))
    {
        LOG_ERROR << "Error reading effect list" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }
    AB::Entities::Version v;
    v.name = "game_effects";
    if (!dataClient_->Read(v))
    {
        LOG_ERROR << "Error reading effect version" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found, "Not found");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";
    declarationNode.append_attribute("standalone") = "yes";
    auto root = doc.append_child("effects");
    root.append_attribute("version") = v.value;

    for (const std::string& uuid : pl.effectUuids)
    {
        AB::Entities::Effect s;
        s.uuid = uuid;
        if (!dataClient_->Read(s))
            continue;
        auto gNd = root.append_child("effect");
        gNd.append_attribute("uuid") = s.uuid.c_str();
        gNd.append_attribute("index") = s.index;
        gNd.append_attribute("name") = s.name.c_str();
        gNd.append_attribute("category") = s.index;
        gNd.append_attribute("icon") = s.icon.c_str();
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
    AB_PROFILE;

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

    AB::Entities::Version v;
    v.name = table;
    if (!dataClient_->Read(v))
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

void Application::GetHandlerStatus(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;

    if (!IsAdmin(request))
    {
        response->write(SimpleWeb::StatusCode::client_error_forbidden,
            "Forbidden");
        return;
    }

    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";
    declarationNode.append_attribute("standalone") = "yes";
    auto root = doc.append_child("status");
    root.append_attribute("version") = "1.0";

    int64_t upTime = Utils::AbTick() - startTime_;
    int64_t mesTime = Utils::AbTick() - statusMeasureTime_;
    {
        auto gNd = root.append_child("bytes_sent");
        gNd.append_attribute("value") = bytesSent_;
    }
    {
        auto gNd = root.append_child("up_time");
        gNd.append_attribute("value") = upTime;
    }
    {
        auto gNd = root.append_child("load");
        gNd.append_attribute("value") = GetAvgLoad();
    }
    {
        auto gNd = root.append_child("bytes_per_second");
        if (mesTime != 0)
            gNd.append_attribute("value") = static_cast<int>(bytesSent_ / (mesTime / 1000));
        else
            gNd.append_attribute("value") = 0;
        gNd.append_attribute("time") = mesTime;
        gNd.append_attribute("round") = uptimeRound_;
    }
    std::stringstream stream;
    doc.save(stream);
    SimpleWeb::CaseInsensitiveMultimap header = GetDefaultHeader();
    header.emplace("Content-Type", "text/xml");
    UpdateBytesSent(stream_size(stream));
    response->write(stream, header);
}

void Application::HandleError(std::shared_ptr<HttpsServer::Request>, const SimpleWeb::error_code& ec)
{
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    if (ec.value() == 995 || ec == SimpleWeb::errc::operation_canceled)
        return;

    LOG_ERROR << "(" << ec.value() << ") " << ec.message() << std::endl;
}
