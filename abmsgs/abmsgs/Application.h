#pragma once

#include "ServerApp.h"
#include "Service.h"
#include "DataClient.h"
#include "MessageServer.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::string configFile_;
    std::string logDir_;
    std::unique_ptr<MessageServer> server_;
    bool running_;
    bool ParseCommandLine();
    void ShowHelp();
    bool LoadMain();
    void PrintServerInfo();
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

