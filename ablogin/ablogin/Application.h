#pragma once

#include "ServerApp.h"
#include "Service.h"

class Application final : public ServerApp
{
private:
    asio::io_service ioService_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    bool LoadMain();
    void PrintServerInfo();
    void HeartBeatTask();
    void ShowLogo();
protected:
    void ShowVersion() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
    std::string GetKeysFile() const;
};

