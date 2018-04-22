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

void TestCreate(DataClient* cli)
{
    // Before Creating a record try Read it to check if such a record already exists.
    LOG_INFO << "TestCreate()" << std::endl;
    AB::Entities::Account a{};
    a.name = "stievie25";
    a.email = "test@test";
    a.password = "test";
    a.uuid = "0d936635-3011-44ec-9df6-da3f25e6fd32";
    {
        AB_PROFILE;
        if (!cli->Create(a))
            LOG_ERROR << "Error Create" << std::endl;
    }
}

void TestReadCache(DataClient* cli)
{
    LOG_INFO << "TestReadCache()" << std::endl;
    AB::Entities::Account a{};
    a.uuid = "0d936635-3011-44ec-9df6-da3f25e6fd32";
    {
        AB_PROFILE;
        if (!cli->Read(a))
            LOG_ERROR << "Error Read" << std::endl;
    }
}

void TestReadDB(DataClient* cli)
{
    LOG_INFO << "TestReadDB()" << std::endl;
    AB::Entities::Account a{};
    a.name = "trill2";
    {
        AB_PROFILE;
        if (!cli->Read(a))
            LOG_ERROR << "Error Read" << std::endl;
    }
}

void TestDelete(DataClient* cli)
{
    LOG_INFO << "TestDelete()" << std::endl;
    AB::Entities::Account a{};
    a.uuid = "0d936635-3011-44ec-9df6-da3f25e6fd32";
    {
        AB_PROFILE;
        if (!cli->Read(a))
        {
            LOG_ERROR << "Error Read" << std::endl;
            return;
        }
    }
    {
        AB_PROFILE;
        if (!cli->Delete(a))
        {
            LOG_ERROR << "Error Delete" << std::endl;
        }
    }
    {
        AB_PROFILE;
        if (!cli->Read(a))
            LOG_ERROR << "Error Read this is correct!" << std::endl;
    }
}

int main()
{
    asio::io_service io_service;
    DataClient cli(io_service);
    cli.Connect("localhost", 2770);

    TestCreate(&cli);
    TestReadCache(&cli);
    TestReadDB(&cli);
    TestDelete(&cli);

    return 0;
}

