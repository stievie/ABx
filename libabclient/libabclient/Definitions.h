#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <sys/timeb.h>
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#define NETWORKMESSAGE_MAXSIZE 16768
// ms = 20 network ticks/second. Update game state each 50ms (= 20 Updates/sec)
#define NETWORK_TICK 50

static constexpr float NETWORK_TICK_MS(static_cast<float>(NETWORK_TICK) / 1000.0f);
