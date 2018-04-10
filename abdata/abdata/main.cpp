// abdata.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <iostream>
#include "Server.h"

static void ShowLogo()
{
    std::cout << "abdata" << std::endl;
    std::cout << std::endl;

    std::cout << "##########  ######  ######" << std::endl;
    std::cout << "    ##          ##  ##" << std::endl;
    std::cout << "    ##  ######  ##  ##" << std::endl;
    std::cout << "    ##  ##      ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;

    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    ShowLogo();
    try
    {
        int port = 2770;
        if (argc > 1)
            port = std::atoi(argv[1]);
        int maxSize = 1024 * 1024 * 1024;
        std::cout << "Port: " << port << std::endl;
        std::cout << "MaxSize: " << maxSize << " bytes" << std::endl;
        asio::io_service io_service;
        Server serv(io_service, (uint16_t)port, maxSize);
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

