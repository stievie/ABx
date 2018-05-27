#pragma once

#include "ServerApp.h"
#include "DataClient.h"
#include "Service.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::string configFile_;
    std::string logDir_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    void ParseCommandLine();
    bool LoadMain();
    void PrintServerInfo();
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;

    IO::DataClient* GetDataClient()
    {
        return dataClient_.get();
    }

    static Application* Instance;
};

