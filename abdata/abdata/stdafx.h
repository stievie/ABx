// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <map>
#include <string>

#define ASIO_STANDALONE

#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#include <uuid.h>

#define WRITE_MINIBUMP
#define _PROFILING

#include "Logger.h"

#define USE_SQLITE
#define USE_MYSQL
#define USE_PGSQL
#define USE_ODBC

// All 24h rotate log
#define LOG_ROTATE_INTERVAL (1000 * 60 * 60 * 24)
#define SCHEDULER_MINTICKS 10
#define MAX_DATA_SIZE (1024 * 1024)
#define MAX_KEY_SIZE 256

// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)

// Number of player names kept in cache
#define MAX_PLAYERNAMES_CACHE 1000
