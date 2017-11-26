// testclient.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <Client.h>
#include <thread>
#include <iostream>
#include "Connection.h"
#include <memory>
#include <mutex>

std::shared_ptr<Client::Client> gClient;
std::mutex gMutex;

int main()
{
    gClient = std::make_shared<Client::Client>();
    bool running = true;
    std::string name;
    std::string pass;
    int charIndex = 0;
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
                gMutex.lock();
                status = 1;
                gMutex.unlock();
            }

            if (status = 2)
            {
                std::lock_guard<std::mutex> lock(gMutex);
                charIndex = std::atoi(input.c_str());
                status = 3;
            }
        }
    }).detach();

    while (running)
    {
        // Game loop
        switch (status)
        {
            case 1:
                gMutex.lock();
                status = 0;
                gMutex.unlock();
                gClient->Login(name, pass);
                break;
            case 3:
            {
                std::string charName = gClient->GetCharacters()[charIndex].name;
                gClient->EnterWorld(charName);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        Client::Connection::Poll();
        switch (gClient->state_)
        {
        case Client::Client::StateSelecChar:
        {
            if (status == 0)
            {
                int i = 0;
                for (const auto& c : gClient->GetCharacters())
                {
                    std::cout << i++ << "\t" << c.name << std::endl;
                }
                gMutex.lock();
                status = 2;
                gMutex.unlock();
            }
            break;
        }
        }
    }
    return 0;
}

