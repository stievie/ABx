#pragma once

#include "Service.h"

class Application
{
private:
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_lock<std::mutex> loaderUniqueLock_;
    Net::ServiceManager serviceManager_;
    std::string configFile_;
    std::string logDir_;
    void MainLoader();
    void PrintServerInfo();
    void ParseCommandLine();
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv);
    void Run();
    void Stop();

    std::string path_;
    std::vector<std::string> arguments_;
};

