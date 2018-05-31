#pragma once

#include "Service.h"
#include "DataClient.h"
#include "ServerApp.h"

class Application : public ServerApp
{
private:
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::string configFile_;
    std::string logDir_;
    bool running_;
    bool LoadMain();
    void PrintServerInfo();
    bool ParseCommandLine();
    void ShowHelp();
    asio::io_service ioService_;
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv);
    void Run();
    void Stop();

    IO::DataClient* GetDataClient()
    {
        return dataClient_.get();
    }

    static Application* Instance;
};
