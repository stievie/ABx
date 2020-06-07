#pragma once

#include <abscommon/ServerApp.h>

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    int64_t lastUpdate_{ 0 };
    bool LoadMain();
    void ShowVersion();
    void ShowLogo();
    void Update();
    void MainLoop();
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
};

