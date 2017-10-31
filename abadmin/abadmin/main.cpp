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

        if (Commands::Instance.Execute(input) == 1)
            std::cout << "OK" << std::endl;
        else
        {
            std::cout << "Failed";
            const std::string& error = gClient->GetErrorMessage();
            if (!error.empty())
                std::cout << ": " << error;
            std::cout << std::endl;
        }
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

    std::cout << "Running, type `h` for some help" << std::endl;

    Run();

    std::cout << "Stopping" << std::endl;

    return EXIT_SUCCESS;
}

