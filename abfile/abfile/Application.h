#pragma once

#include "ServerApp.h"
#include <boost/filesystem.hpp>

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

namespace IO {
class DataClient;
}

class Application : public ServerApp, public std::enable_shared_from_this<Application>
{
private:
    std::unique_ptr<HttpsServer> server_;
    std::string root_;
    std::string configFile_;
    std::string logDir_;
    std::unique_ptr<IO::DataClient> dataClient_;
    asio::io_service ioService_;
    std::string dataHost_;
    uint16_t dataPort_;
    bool running_;
    bool IsAllowed(std::shared_ptr<HttpsServer::Request> request);
    static bool IsHidden(const boost::filesystem::path& path);
    void GetHandlerDefault(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerGames(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerSkills(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerProfessions(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerAttributes(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void GetHandlerVersion(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void HandleError(std::shared_ptr<HttpsServer::Request> /*request*/,
        const SimpleWeb::error_code & /*ec*/);
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

