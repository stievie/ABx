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

void TestUpdate(DataClient* cli)
{
    LOG_INFO << "TestUpdate()" << std::endl;
    AB::Entities::Account a{};
    a.name = "trill2";
    {
        if (!cli->Read(a))
            LOG_ERROR << "Error Read" << std::endl;
        else
        {
            int c = 0;
            while (c < 10)
            {
                ++c;
                AB_PROFILE;
                a.email = "test@test.2";
                if (!cli->Update(a))
                {
                    LOG_ERROR << "Error Update" << std::endl;
                    break;
                }
            }
        }
    }

}

void TestPreload(DataClient* cli)
{
    LOG_INFO << "TestUpdate()" << std::endl;
    AB::Entities::Account a{};
    a.name = "stievie7";
    {
        AB_PROFILE;
        if (!cli->Preload(a))
        {
            LOG_ERROR << "Error Preload" << std::endl;
            return;
        }
    }
    {
        AB_PROFILE;
        if (!cli->Read(a))
            LOG_ERROR << "Error Read" << std::endl;
    }
    {
        AB_PROFILE;
        if (!cli->Invalidate(a))
            LOG_ERROR << "Error Invalidate" << std::endl;
    }
    {
        AB_PROFILE;
        if (!cli->Read(a))
            LOG_ERROR << "Error Read" << std::endl;
    }
}

int main()
{
    std::cout << "Connecting..." << std::endl;
    asio::io_service io_service;
    DataClient cli(io_service);
    cli.Connect("localhost", 2770);
    if (!cli.IsConnected())
    {
        std::cout << "Failed to connect to server" << std::endl;
        return 1;
    }

run_it:
    TestCreate(&cli);
    TestReadCache(&cli);
    TestReadDB(&cli);
    TestDelete(&cli);
    TestUpdate(&cli);
    TestPreload(&cli);

    std::cout << "Run again? [y/n]: ";
    std::string answer;
    std::cin >> answer;
    if (answer.compare("y") == 0)
    {
        goto run_it;
    }

    return 0;
}

