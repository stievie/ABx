// abserv.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "Service.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ProtocolLogin.h"
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

    // ENC ------------------------
    LOG_INFO << "Loading RSA key...";
    const char* p("MIICWgIBAAKBgFm8OdzKgKgOXkWrndXFqmzuHjRbUTTgWn87Izl0MdipcKghEyyljcD4Z04Msg+N0r91ChbqeswreJvdGItQvtEwrHgtTTaDzmELk5XbEmVkxHw1CNwcJh+HR8dJjUV8JA3OSqnzE4tGTf6");
    const char* q("i5NBdWXA2gBQ9qTAbhSMt1ZyC6JiuKt2xzmMs7IFh3y0QQJBAJztZyrChf08T4v8snOEl2aKNoJqHFSZUhgK+yW0h5LDgdrwda9fC/3rn9dBP5hONTHIi4ZXy7hT8X32/Cevz3kCQQCSYzy2fbmHOQqyfnF");
    const char* d("snOEl2aKNoJqHFSZUhgK+yW0h5LDgdrwda9fC/3rn9dBP5hONTHIi4ZXy7hT8X32/Cevz3kCQQCSYzy2fbmHOKK64JB9aqQqyfnFh8nVMPD6UacTuPQUAaUI5z4GtDyeA/sLziasMu7TYENuHbZ0dTaJl9Xj9kp5AkAYe8251Sm0jeFXVPC+pzQ78lp41HdhF57AU45Fnrn8QvaSoyupVen4DvgcTHjQmXshLknehvoo4yftYEiNJJf5AkAmcrQWhkz9TA3JoYOxvRmjN2tHy1NikDkqtdl5H6HTw17SSNIMtrgZFJiSUDHuFm6NzAHJ4Tnz");
    Rsa::Instance.SetKey(p, q, d);
    LOG_INFO << "[done]" << std::endl;


    serviceManager->Add<ProtocolGame>(ConfigManager::Instance.config_[ConfigManager::Key::GamePort].GetInt());
    serviceManager->Add<ProtocolLogin>(ConfigManager::Instance.config_[ConfigManager::Key::LoginPort].GetInt());

    PrintServerInfo(serviceManager);


    LOG_INFO << "Loading done in " << (AbTick() - startLoading) / (1000) << " sec." << std::endl;

    GameManager::Instance.Start(serviceManager);

    // notify we are ready
    loaderSignal.notify_all();
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

    Scheduler::Instance.Stop();
    Scheduler::Instance.Terminate();
    Dispatcher::Instance.Stop();
    Dispatcher::Instance.Terminate();

    return 0;
}
