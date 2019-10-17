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

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4592)
#endif
#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wpadded"
#endif
#if defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpadded"
#endif
#include <asio.hpp>
#if defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#   pragma clang diagnostic pop
#endif
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4702 4127)
#endif
#include <lua.hpp>
#include <kaguya/kaguya.hpp>
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4201 4267 4244)
#endif
#include <ai/SimpleAI.h>
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#include <base64.h>
#include <uuid.h>

#include "Utils.h"
#include <sa/Events.h>
#include "Profiler.h"
#include "Mechanic.h"
#include "Application.h"
