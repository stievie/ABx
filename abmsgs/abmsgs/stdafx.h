// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>

#include <stdint.h>
#include <cassert>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>

#include "DebugConfig.h"

#define ASIO_STANDALONE
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#include <uuid.h>

#include "Logger.h"

#define SCHEDULER_MINTICKS 10

#define WRITE_MINIBUMP
//#define _PROFILING
// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
