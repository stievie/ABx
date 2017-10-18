#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "Service.h"

class Application
{
private:
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_lock<std::mutex> loaderUniuqueLock_;
    Net::ServiceManager serviceManager_;
    void MainLoader();
    void PrintServerInfo();
public:
    Application();
    ~Application() {}

    bool Initialize(int argc, char** argv);
    void Run();

    std::string path_;
    std::vector<std::string> arguments_;
};

