#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <list>
#include "Definitions.h"
#include <string>

static void ShowHelp()
{

}

static void Run()
{
    while (true)
    {
        std::cout << "AB> ";
        std::string input;
        std::cin >> input;

        if (input.compare("q") == 0)
            break;
        else if (input.compare("h") == 0)
            ShowHelp();
    }
}

int main()
{
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

