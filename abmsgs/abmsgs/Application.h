#pragma once

#include "ServerApp.h"
#include "Service.h"
#include "DataClient.h"
#include "MessageServer.h"
#include "IpList.h"

constexpr int64_t QUEUE_UPDATE_INTERVAL_MS = 1000;

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::unique_ptr<MessageServer> server_;
    Net::IpList whiteList_;
    int64_t lastUpdate_{ 0 };
    void ShowHelp();
    bool LoadMain();
    void PrintServerInfo();
    void UpdateQueue();
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
};

