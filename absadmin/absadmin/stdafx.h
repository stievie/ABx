// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#if defined(_MSC_VER)
// Decorated name length exceeded
#pragma warning(disable: 4503 4307)
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

#include <stdio.h>
#include <stdint.h>

#include <memory>
#include <limits>
#include <unordered_map>

#include <AB/CommonConfig.h>
#include "DebugConfig.h"

#if !defined(USE_STANDALONE_ASIO)
#define USE_STANDALONE_ASIO
#endif
#include "Servers.h"

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4701 4800)
#   include <ginger.h>
PRAGMA_WARNING_POP

#include <json.hpp>

#include "Utils.h"

#define WRITE_MINIBUMP
#define PROFILING
