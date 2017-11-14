#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <sys/timeb.h>
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#define NETWORKMESSAGE_MAXSIZE 16768

inline void AB_SLEEP(uint32_t t)
{
    Sleep(t);
}

inline long long AB_TIME()
{
    _timeb t;
    _ftime64_s(&t);
    return ((long long)t.millitm) + ((long long)t.time) * 1000;
}
