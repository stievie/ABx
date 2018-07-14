#pragma once

#include "ServerApp.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    bool running_;
    std::string localHost_;
    uint16_t localPort_;
    std::string remoteHost_;
    uint16_t remotePort_;
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

