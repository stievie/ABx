#include "stdafx.h"
#include "Application.h"
#include "SimpleConfigManager.h"
#include <boost/filesystem.hpp>
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
#include <pugixml.hpp>
#include "Profiler.h"

Application::Application() :
    ServerApp::ServerApp(),
    running_(false),
    ioService_()
{
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

    if (configFile_.empty())
        configFile_ = path_ + "/abfile.lua";
    IO::SimpleConfigManager::Instance.Load(configFile_);

    std::string address = IO::SimpleConfigManager::Instance.GetGlobal("server_ip", "");
    uint16_t port = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("server_port", 8081));
    std::string key = IO::SimpleConfigManager::Instance.GetGlobal("server_key", "server.key");
    std::string cert = IO::SimpleConfigManager::Instance.GetGlobal("server_cert", "server.crt");
    size_t threads = IO::SimpleConfigManager::Instance.GetGlobal("num_threads", 1);
    root_ = IO::SimpleConfigManager::Instance.GetGlobal("root_dir", "");
    logDir_ = IO::SimpleConfigManager::Instance.GetGlobal("log_dir", "");
    dataHost_ = IO::SimpleConfigManager::Instance.GetGlobal("data_host", "localhost");
    dataPort_ = static_cast<uint16_t>(IO::SimpleConfigManager::Instance.GetGlobal("data_port", 2770));

    if (!logDir_.empty() && logDir_.compare(IO::Logger::logDir_) != 0)
    {
        // Different log dir
        IO::Logger::logDir_ = logDir_;
        IO::Logger::Close();
    }

    server_ = std::make_unique<HttpsServer>(cert, key);
    server_->config.port = port;
    server_->config.thread_pool_size = threads;

    server_->default_resource["GET"] = std::bind(&Application::GetHandlerDefault, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    server_->resource["^/_version_$"]["GET"] = std::bind(&Application::GetHandlerVersion, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_games_$"]["GET"] = std::bind(&Application::GetHandlerGames, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_skills_$"]["GET"] = std::bind(&Application::GetHandlerSkills, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);
    server_->resource["^/_professions_$"]["GET"] = std::bind(&Application::GetHandlerProfessions, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    server_->on_error = std::bind(&Application::HandleError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2);

    dataClient_ = std::make_unique<IO::DataClient>(ioService_);

    LOG_INFO << "Connecting to data server...";
    dataClient_->Connect(dataHost_, dataPort_);
    if (!dataClient_->IsConnected())
    {
        LOG_ERROR << "Failed to connect to data server" << std::endl;
        return false;
    }
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Server config:" << std::endl;
    LOG_INFO << "  Config file: " << (configFile_.empty() ? "(empty)" : configFile_) << std::endl;
    LOG_INFO << "  Listening: " << (address.empty() ? "0.0.0.0" : address) << ":" << port << std::endl;
    LOG_INFO << "  Log dir: " << (IO::Logger::logDir_.empty() ? "(empty)" : IO::Logger::logDir_) << std::endl;
    LOG_INFO << "  Data Server: " << dataClient_->GetHost() << ":" << dataClient_->GetPort() << std::endl;

    return true;
}

void Application::Run()
{
    running_ = true;
    LOG_INFO << "Server is running" << std::endl;
    server_->start();
}

void Application::Stop()
{
    running_ = false;
    LOG_INFO << "Server shutdown..." << std::endl;
    server_->stop();
}

void Application::GetHandlerDefault(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    try
    {
        auto web_root_path = boost::filesystem::canonical(root_);
        auto path = boost::filesystem::canonical(web_root_path / request->path);
        // Check if path is within web_root_path
        if (std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
            !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
            throw std::invalid_argument("path must be within root path");
        if (boost::filesystem::is_directory(path))
            throw std::invalid_argument("not a file");

        SimpleWeb::CaseInsensitiveMultimap header;

        // Uncomment the following line to enable Cache-Control
        // header.emplace("Cache-Control", "max-age=86400");

        auto ifs = std::make_shared<std::ifstream>();
        ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

        if (*ifs)
        {
            auto length = ifs->tellg();
            ifs->seekg(0, std::ios::beg);

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
    catch (const std::exception &e)
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request->path);
    }
}

//void Application::GetHandlerInfo(std::shared_ptr<HttpsServer::Response> response,
//    std::shared_ptr<HttpsServer::Request> request)
//{
//    std::stringstream stream;
//    stream << "<h1>Request from " << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << "</h1>";
//
//    stream << request->method << " " << request->path << " HTTP/" << request->http_version;
//
//    stream << "<h2>Query Fields</h2>";
//    auto query_fields = request->parse_query_string();
//    for (auto &field : query_fields)
//        stream << field.first << ": " << field.second << "<br>";
//
//    stream << "<h2>Header Fields</h2>";
//    for (auto &field : request->header)
//        stream << field.first << ": " << field.second << "<br>";
//
//    response->write(stream);
//}

void Application::GetHandlerGames(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;
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
    response->write(stream);
}

void Application::GetHandlerSkills(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;
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
    response->write(stream);
}

void Application::GetHandlerProfessions(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    AB_PROFILE;
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
    response->write(stream);
}

void Application::GetHandlerVersion(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
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
    response->write(stream);
}

void Application::HandleError(std::shared_ptr<HttpsServer::Request>, const SimpleWeb::error_code&)
{
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
}
