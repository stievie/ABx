// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

PRAGMA_WARNING_DISABLE_MSVC(4307)

#include <stdio.h>
#include <cassert>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>

#include <AB/CommonConfig.h>
#include "DebugConfig.h"

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif
PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
#   include <asio.hpp>
PRAGMA_WARNING_POP

#include <uuid.h>

#define WRITE_MINIBUMP
#define AB_UNUSED(P) (void)(P)

#include "Utils.h"
#include "Logger.h"

// Maximum connections to this server. A single machine can maybe handle up to 3000 concurrent connections.
// https://www.gamedev.net/forums/topic/319003-mmorpg-and-the-ol-udp-vs-tcp/?do=findComment&comment=3052256
#define SERVER_MAX_CONNECTIONS 3000

// When a user deletes a character, the characters name is reserved for 1 week
static constexpr int NAME_RESERVATION_EXPIRES_MS = 1000 * 60 * 60 * 24 * 7;
