#pragma once

#include "ServerApp.h"

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

class Application : public ServerApp, public std::enable_shared_from_this<Application>
{
private:
    std::unique_ptr<HttpsServer> server_;
    std::string root_;
    std::string configFile_;
    bool running_;
    void DefaultGetHandler(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
    void InfoGetHandler(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

