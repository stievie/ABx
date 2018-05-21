// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>

#include <memory>

#define USE_STANDALONE_ASIO
#pragma warning(push)
#pragma warning(disable: 4592)
#include <SimpleWeb/server_https.hpp>
#pragma warning(pop)

// Decorated name length exceeded
#pragma warning(disable: 4503)

#define _PROFILING
// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
