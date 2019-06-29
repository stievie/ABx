#pragma once

#include "ServerApp.h"
#include "Service.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    void ShowHelp();
    bool LoadMain();
    void PrintServerInfo();
    void HeardBeatTask();
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
    std::string GetKeysFile() const;
};

