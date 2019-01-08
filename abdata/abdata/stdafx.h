// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>

#include "DebugConfig.h"

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <asio.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <uuid.h>

#define WRITE_MINIBUMP
#define _PROFILING

#include "Logger.h"

#define USE_SQLITE
#define USE_MYSQL
#define USE_PGSQL
#ifdef _WIN32
#define USE_ODBC
#endif

#define SCHEDULER_MINTICKS 10
#define MAX_DATA_SIZE (1024 * 1024)
#define MAX_KEY_SIZE 256

// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
