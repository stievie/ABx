#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <list>
#include "Definitions.h"

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

    return EXIT_SUCCESS;
}

