/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

PRAGMA_WARNING_DISABLE_MSVC(4307)

#include <stdio.h>

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif

#define AB_UNUSED(P) (void)(P)

#include <sa/Assert.h>
#include <AB/CommonConfig.h>
#include <abscommon/ServiceConfig.h>
#include "Config.h"
#include <abscommon/DebugConfig.h>

#include <cmath>

// STL
#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>

#include <eastl.hpp>

#include <limits>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define BRIGAND_NO_BOOST_SUPPORT
#include <absmath/MathConfig.h>
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

#include <abshared/Mechanic.h>
#include <abshared/Attributes.h>

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/GuildMembers.h>
#include <AB/Entities/Mail.h>
#include <AB/Entities/MailList.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/Logger.h>
#include <abscommon/NetworkMessage.h>
#include <abscommon/OutputMessage.h>
#include <abscommon/Profiler.h>
#include <abscommon/Random.h>
#include <abscommon/Scheduler.h>
#include <abscommon/Subsystems.h>
#include <absmath/Vector3.h>
#include <base64.h>
#include <sa/CallableTable.h>
#include <sa/CircularQueue.h>
#include <sa/Events.h>
#include <sa/IdGenerator.h>
#include <sa/Iteration.h>
#include <sa/PropStream.h>
#include <sa/StringHash.h>
#include <sa/StrongType.h>
#include <sa/TypeName.h>
#include <sa/WeightedSelector.h>
#include <uuid.h>
