// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen

#include <memory>
#include <iostream>

#define ASIO_STANDALONE

#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#include <uuid.h>

#define SCHEDULER_MINTICKS 10

// Maximum connections to this server. A single machine can maybe handle up to 3000 concurrent connections.
// https://www.gamedev.net/forums/topic/319003-mmorpg-and-the-ol-udp-vs-tcp/?do=findComment&comment=3052256
#define SERVER_MAX_CONNECTIONS 3000

#define AB_UNUSED(P) (void)(P)
