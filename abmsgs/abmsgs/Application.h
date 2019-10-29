#pragma once

#include "ServerApp.h"
#include "Service.h"
#include "DataClient.h"
#include "MessageServer.h"
#include "IpList.h"

class Application final : public ServerApp
{
private:
    asio::io_service ioService_;
    std::unique_ptr<MessageServer> server_;
    Net::IpList whiteList_;
    int64_t lastUpdate_{ 0 };
    bool LoadMain();
    void PrintServerInfo();
    void ShowLogo();
protected:
    void ShowVersion() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
};

