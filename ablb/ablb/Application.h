#pragma once

#include "ServerApp.h"
#include "DataClient.h"
#include "Acceptor.h"
#include <AB/Entities/Service.h>

class Application : public ServerApp
{
private:
    using ServiceItem = std::pair<std::string, uint16_t>;
    std::vector<ServiceItem> serviceList_;
    asio::io_service ioService_;
    bool running_;
    std::string localHost_;
    uint16_t localPort_;
    AB::Entities::ServiceType lbType_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::unique_ptr<Acceptor> acceptor_;
    std::string logDir_;
    std::string configFile_;
    bool ParseCommandLine();
    void ShowHelp();
    void PrintServerInfo();
    bool LoadMain();
    bool GetServiceCallback(AB::Entities::Service& svc);
    bool GetServiceCallbackList(AB::Entities::Service& svc);
    bool ParseServerList(const std::string& fileName);
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

