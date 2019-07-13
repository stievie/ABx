// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"

#include <stdio.h>

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#define WRITE_MINIBUMP

#include <AB/CommonConfig.h>
#include "DebugConfig.h"

#define ASIO_STANDALONE

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <asio.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "Utils.h"
#include "Logger.h"

// Used by the profiler to generate a unique identifier
#define CONCAT(a, b) a ## b
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
