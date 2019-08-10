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

#if !defined(USE_STANDALONE_ASIO)
#define USE_STANDALONE_ASIO
#endif
#include "Servers.h"

#include <stdio.h>
#include <stdint.h>

#include <memory>
#include <limits>
#include <iostream>

#include <pugixml.hpp>
#include <abcrypto.hpp>
#include <AB/CommonConfig.h>
#include "DebugConfig.h"
#include "Utils.h"
#include "Profiler.h"

#define WRITE_MINIBUMP
#define PROFILING
