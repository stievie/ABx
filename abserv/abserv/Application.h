#pragma once

#include "Service.h"
#include "DataClient.h"

class Application
{
private:
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_lock<std::mutex> loaderUniqueLock_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::string configFile_;
    std::string logDir_;
    void MainLoader();
    void PrintServerInfo();
    void ParseCommandLine();
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

    std::string path_;
    std::vector<std::string> arguments_;

    static Application* Instance;
};
