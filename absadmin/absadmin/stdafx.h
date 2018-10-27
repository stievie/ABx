// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#if defined(_MSC_VER)
// Decorated name length exceeded
#pragma warning(disable: 4503)
#endif

#include "targetver.h"

#include <stdio.h>
#include <stdint.h>

#include <memory>
#include <limits>

#include "DebugConfig.h"

#if !defined(USE_STANDALONE_ASIO)
#define USE_STANDALONE_ASIO
#endif
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <SimpleWeb/server_https.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4701 4800)
#endif
#include <ginger.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <json.hpp>

#define WRITE_MINIBUMP
#define _PROFILING

//#define _PROFILING
// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
