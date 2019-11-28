// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4307)
#endif

#include <stdio.h>

#define ASIO_STANDALONE

#define AB_UNUSED(P) (void)(P)

#include <cassert>
#include <AB/CommonConfig.h>
#include <sa/PragmaWarning.h>
#include "ServiceConfig.h"
#include "Config.h"
#include "DebugConfig.h"

#include <cmath>

// STL
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <ostream>
#include <iostream>
#include <sstream>
#include <functional>
#include <condition_variable>
#include <unordered_set>
#include <forward_list>

#include <limits>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define WRITE_MINIBUMP

#include "MathConfig.h"
#include <pugixml.hpp>

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
    PRAGMA_WARNING_DISABLE_CLANG("-Wpadded")
    PRAGMA_WARNING_DISABLE_GCC("-Wpadded")
#   include <asio.hpp>
PRAGMA_WARNING_POP

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4702 4127)
#   include <lua.hpp>
#   include <kaguya/kaguya.hpp>
PRAGMA_WARNING_POP

#include <base64.h>
#include <uuid.h>

#include "Utils.h"
#include <sa/Events.h>
#include "Profiler.h"
#include "Mechanic.h"
#include "Application.h"
