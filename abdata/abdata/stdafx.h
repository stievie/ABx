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

#include <iostream>
#include <map>
#include <string>

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
#include <base64.h>

#define WRITE_MINIBUMP
#define PROFILING

#include "Logger.h"
#include "UuidUtils.h"

#define MAX_DATA_SIZE (1024 * 1024)
#define MAX_KEY_SIZE 256

#if !defined(USE_MYSQL) && !defined(USE_PGSQL) && !defined(USE_ODBC) && !defined(USE_SQLITE)
#error "Define at least one database driver"
#endif
