// abserv.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "Service.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ConfigManager.h"
#include "Task.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include "Logger.h"

std::mutex loaderLock;
std::condition_variable loaderSignal;
std::unique_lock<std::mutex> loaderUniuqueLock(loaderLock);

static void MainLoader(ServiceManager* serviceManager)
{
    LOG_INFO << "Loading..." << std::endl;

    serviceManager->Add<ProtocolGame>(ConfigManager::Instance.config_[ConfigManager::Key::GamePort].GetInt());

    // notify we are ready
    loaderSignal.notify_all();

    LOG_INFO << "Loading done" << std::endl;
}

int main(int argc, char** argv)
{
    ServiceManager serviceManager;

    Dispatcher::Instance.Start();
    Scheduler::Instance.Start();

    Dispatcher::Instance.Add(CreateTask(std::bind(MainLoader, &serviceManager)));
    loaderSignal.wait(loaderUniuqueLock);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    if (serviceManager.IsRunning())
    {
        LOG_INFO << "Server is running" << std::endl;
        serviceManager.Run();
    }
    else
    {
        // No services running
        LOG_ERROR << "No services running" << std::endl;
    }

    Dispatcher::Instance.Stop();
    Dispatcher::Instance.Terminate();

    return 0;
}
