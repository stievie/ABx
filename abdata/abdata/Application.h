#pragma once

#include "ServerApp.h"
#include "Server.h"

class Application : public ServerApp
{
private:
    uint16_t port_;
    uint32_t listenIp_;
    size_t maxSize_;
    std::string configFile_;
    bool readonly_;
    bool running_;
    asio::io_service ioService_;
    std::unique_ptr<Server> server_;
    uint32_t flushInterval_;
    uint32_t cleanInterval_;
    bool ParseCommandLine();
    bool LoadConfig();
    void PrintServerInfo();
    void ShowHelp();
public:
    Application() :
        ServerApp::ServerApp(),
        port_(0),
        listenIp_(0),
        maxSize_(0),
        readonly_(false),
        running_(false),
        ioService_(),
        server_(nullptr),
        flushInterval_(FLUSH_CACHE_MS),
        cleanInterval_(CLEAN_CACHE_MS)
    { }
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

