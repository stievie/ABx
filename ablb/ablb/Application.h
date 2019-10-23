#pragma once

#include "ServerApp.h"
#include "DataClient.h"
#include "Acceptor.h"
#include <AB/Entities/Service.h>

class Application : public ServerApp
{
private:
    typedef std::pair<std::string, uint16_t> ServiceItem;
    std::vector<ServiceItem> serviceList_;
    asio::io_service ioService_;
    AB::Entities::ServiceType lbType_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::unique_ptr<Acceptor> acceptor_;
    void PrintServerInfo();
    bool LoadMain();
    bool GetServiceCallback(AB::Entities::Service& svc);
    bool GetServiceCallbackList(AB::Entities::Service& svc);
    bool ParseServerList(const std::string& fileName);
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
};

