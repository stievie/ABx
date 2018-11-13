#pragma once

#include "ServerApp.h"
#include "Server.h"

class Application : public ServerApp
{
private:
    uint32_t listenIp_;
    size_t maxSize_;
    bool readonly_;
    asio::io_service ioService_;
    std::unique_ptr<Server> server_;
    uint32_t flushInterval_;
    uint32_t cleanInterval_;
    bool LoadConfig();
    void PrintServerInfo();
    void ShowHelp();
protected:
    bool ParseCommandLine();
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

