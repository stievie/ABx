#pragma once

#include "ServerApp.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    bool running_;
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

