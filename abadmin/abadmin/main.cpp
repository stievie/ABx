#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <list>
#include "Definitions.h"
#include "NetworkMessage.h"
#include <string>
#include "Client.h"
#include "Commands.h"
#include <time.h>

Client* gClient = nullptr;
bool gRunning = false;

static void Run()
{
    Client cli;
    gClient = &cli;
    Commands::Instance.Initialize();

    gRunning = true;
    while (gRunning)
    {
        std::cout << "AB";
        if (cli.IsConnected() && cli.IsLoggedIn())
        {
            std::cout << " " << cli.GetHost() << ":" << cli.GetPort();
        }
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);

        Commands::Instance.Execute(input);
    }
    gClient = nullptr;
}

static void ShowLogo()
{
    std::cout << "##########  ######  ######" << std::endl;
    std::cout << "    ##          ##  ##" << std::endl;
    std::cout << "    ##  ######  ##  ##" << std::endl;
    std::cout << "    ##  ##      ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
}

int main()
{
    ShowLogo();
    std::cout << std::endl;
    srand((unsigned)time(NULL));

#if defined WIN32 || defined __WINDOWS__
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        std::cout << "WSAStartup() failed" << std::endl;
        return EXIT_FAILURE;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    srand(counter.LowPart);
#else
    srand(time(NULL));
#endif
    std::cout << "Running, type `h` for some help" << std::endl;

    Run();

    std::cout << "Stopping" << std::endl;

#if defined WIN32 || defined __WINDOWS__
    WSACleanup();
#endif

    return EXIT_SUCCESS;
}

