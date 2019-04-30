// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <cassert>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>

#include "DebugConfig.h"

#define ASIO_STANDALONE
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4592)
#endif // defined
#include <asio.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <uuid.h>

#define WRITE_MINIBUMP
#define AB_UNUSED(P) (void)(P)

#include "Logger.h"

#define SCHEDULER_MINTICKS 10
// Maximum connections to this server. A single machine can maybe handle up to 3000 concurrent connections.
// https://www.gamedev.net/forums/topic/319003-mmorpg-and-the-ol-udp-vs-tcp/?do=findComment&comment=3052256
#define SERVER_MAX_CONNECTIONS 3000

// When a user deletes a character, the characters name is reserved for 1 week
static constexpr int NAME_RESERVATION_EXPIRES_MS = 1000 * 60 * 60 * 24 * 7;

#include <AB/CommonConfig.h>
