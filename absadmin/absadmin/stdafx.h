// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#if defined(_MSC_VER)
// Decorated name length exceeded
#pragma warning(disable: 4503)
#endif

#include "targetver.h"

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

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4701 4800)
#endif
#include <ginger.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <json.hpp>

#include "Utils.h"

#define WRITE_MINIBUMP
#define PROFILING
