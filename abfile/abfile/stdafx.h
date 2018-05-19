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
#pragma warning(disable: 4267 4592 4458 4457 4456)
#include <SimpleWeb/server_https.hpp>
#pragma warning(pop)

#define _PROFILING
// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
