// abserv.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "Service.h"
#include "Dispatcher.h"

int main(int argc, char** argv)
{
    ServiceManager serviceManager;

    Dispatcher::Instance.Start();
    Scheduler::Instance.Start();

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

