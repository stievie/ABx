// testclient.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <Client.h>
#include <thread>
#include <iostream>
#include "Connection.h"

Client::Client gClient;

int main()
{

    bool running = true;
    std::string name;
    std::string pass;
    int status = 0;
    std::thread([&]() {
        while (running)
        {
            std::cout << "AB";
            std::cout << "> ";
            std::string input;
            std::getline(std::cin, input);

            if (input.compare("q") == 0)
                running = false;
            else if (input.compare("login") == 0)
            {
                std::cout << "Name: ";
                std::getline(std::cin, name);
                std::cout << "Pass: ";
                std::getline(std::cin, pass);
                if (name.empty() || pass.empty())
                {
                    continue;
                }
                status = 1;
            }
        }
    }).detach();

    while (running)
    {
        // Game loop
        switch (status)
        {
            case 1:
                status = 0;
                gClient.Login(name, pass);
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        Client::Connection::Poll();
    }
    return 0;
}

