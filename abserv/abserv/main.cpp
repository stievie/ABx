// abserv.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "Service.h"
#include "Dispatcher.h"
#include "ProtocolGame.h"
#include "ConfigManager.h"
#include "Task.h"

static void MainLoader(ServiceManager* serviceManager)
{
    serviceManager->Add<ProtocolGame>(ConfigManager::Instance.config_[ConfigManager::Key::GamePort].GetInt());
}

int main(int argc, char** argv)
{
    ServiceManager serviceManager;

    Dispatcher::Instance.Start();
    Scheduler::Instance.Start();

    Dispatcher::Instance.Add(CreateTask(std::bind(MainLoader, &serviceManager)));

    if (serviceManager.IsRunning())
    {
        serviceManager.Run();
    }
    else
    {
        // No services running
    }

    return 0;
}
