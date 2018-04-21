// datatest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include "DataClient.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include <uuid.h>
#define _PROFILING
#include "Profiler.h"

int main()
{
    asio::io_service io_service;
    DataClient cli(io_service);
    cli.Connect("localhost", 2770);
    AB::Entities::Account a{ };

    uuids::uuid empty;
    a.uuid = empty.to_string();
    a.name = "trill";
    {
        AB_PROFILE;
        cli.Read(a);
    }
    std::cout << a.email << std::endl;

    AB::Entities::Account a2{};
    a2.uuid = a.uuid;
    {
        AB_PROFILE;
        cli.Read(a2);
    }
    std::cout << a2.email << std::endl;
    a2.email = "test";
    {
        AB_PROFILE;
        cli.Update(a2);
    }
    std::cout << a2.email << std::endl;

    AB::Entities::Account a4{};
    a4.name = "stievie25";
    a4.email = "test@test";
    a4.uuid = uuids::uuid_system_generator{}().to_string();
    {
        AB_PROFILE;
        cli.Create(a4);
    }

    AB::Entities::Account a5{};
    a5.uuid = a4.uuid;
    {
        AB_PROFILE;
        cli.Read(a5);
    }

    AB::Entities::Account a3{};
    a3.uuid = a5.uuid;
    {
        AB_PROFILE;
        cli.Read(a3);
    }
    std::cout << a3.email << std::endl;

    {
        AB_PROFILE;
        cli.Delete(a3);
    }
    if (!cli.Read(a3))
        std::cout << "noexists" << std::endl;

    return 0;
}

