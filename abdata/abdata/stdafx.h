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

#define USE_SQLITE
#define USE_MYSQL
#define USE_PGSQL
#define USE_ODBC

#define SCHEDULER_MINTICKS 10
#define MAX_DATA_SIZE (1024 * 1024)
#define MAX_KEY_SIZE 256
