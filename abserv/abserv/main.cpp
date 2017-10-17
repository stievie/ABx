// abserv.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "Service.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ProtocolLogin.h"
#include "ProtocolAdmin.h"
#include "ProtocolStatus.h"
#include "ConfigManager.h"
#include "Task.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include "Logger.h"
#include "System.h"
#include "GameManager.h"
#include "Rsa.h"

std::mutex loaderLock;
std::condition_variable loaderSignal;
std::unique_lock<std::mutex> loaderUniuqueLock(loaderLock);
std::string appPath;

static void PrintServerInfo(ServiceManager* serviceManager)
{
    std::list<uint16_t> ports = serviceManager->GetPorts();
    LOG_INFO << "Local ports: ";
    while (ports.size())
    {
        LOG_INFO << ports.front() << "\t";
        ports.pop_front();
    }
    LOG_INFO << std::endl;
}

static void MainLoader(ServiceManager* serviceManager)
{
    int64_t startLoading = AbTick();

    LOG_INFO << "Loading..." << std::endl;

    LOG_INFO << "Loading configuration...";
    ConfigManager::Instance.Load(appPath + "/" + CONFIG_FILE);
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Initializing PRNG...";
    std::srand((unsigned)AbTick());
    LOG_INFO << "[done]" << std::endl;

    // DB ----------------
    LOG_INFO << "Creating DB connection...";
    // TODO:
    LOG_INFO << "[done]" << std::endl;

    LOG_INFO << "Checking DB schema version...";
    // TODO:
    LOG_INFO << "[done]" << std::endl;



    serviceManager->Add<ProtocolLogin>(ConfigManager::Instance.config_[ConfigManager::Key::LoginPort].GetInt());
    serviceManager->Add<ProtocolAdmin>(ConfigManager::Instance.config_[ConfigManager::Key::AdminPort].GetInt());
    serviceManager->Add<ProtocolStatus>(ConfigManager::Instance.config_[ConfigManager::Key::StatusPort].GetInt());
    serviceManager->Add<ProtocolGame>(ConfigManager::Instance.config_[ConfigManager::Key::GamePort].GetInt());

    PrintServerInfo(serviceManager);


    LOG_INFO << "Loading done in " << (AbTick() - startLoading) / (1000) << " sec." << std::endl;

    GameManager::Instance.Start(serviceManager);

    // Notify we are ready
    loaderSignal.notify_all();
}

int main(int argc, char** argv)
{
    std::string aux(argv[0]);
    size_t pos = aux.find_last_of("\\/");
    appPath = aux.substr(0, pos);

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

    Scheduler::Instance.Stop();
    Scheduler::Instance.Terminate();
    Dispatcher::Instance.Stop();
    Dispatcher::Instance.Terminate();

    return 0;
}
