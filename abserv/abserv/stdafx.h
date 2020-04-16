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

#define ASIO_STANDALONE

#define AB_UNUSED(P) (void)(P)

#include <cassert>
#include <AB/CommonConfig.h>
#include <abscommon/ServiceConfig.h>
#include "Config.h"
#include <abscommon/DebugConfig.h>

#include <cmath>

// STL
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <forward_list>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <limits>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define WRITE_MINIBUMP

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

#include "Actor.h"
#include "AiAgent.h"
#include "Application.h"
#include "CollisionComp.h"
#include <abshared/Damage.h>
#include "Game.h"
#include "GameObject.h"
#include "Map.h"
#include "Player.h"
#include "PlayerManager.h"
#include "Script.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/GuildMembers.h>
#include <AB/Entities/Mail.h>
#include <AB/Entities/MailList.h>
#include <AB/Packets/ClientPackets.h>
#include <AB/ProtocolCodes.h>
#include <abai/Agent.h>
#include <abscommon/BanManager.h>
#include <abscommon/Connection.h>
#include <abscommon/CpuUsage.h>
#include <abscommon/DataClient.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Logger.h>
#include <abscommon/MessageClient.h>
#include <abscommon/MessageMsg.h>
#include <abscommon/NetworkMessage.h>
#include <abscommon/OutputMessage.h>
#include <abscommon/Profiler.h>
#include <abscommon/Random.h>
#include <abscommon/Scheduler.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/ThreadPool.h>
#include <abscommon/Utils.h>
#include <abscommon/UuidUtils.h>
#include <abscommon/Variant.h>
#include <absmath/BoundingBox.h>
#include <absmath/CollisionShape.h>
#include <absmath/ConvexHull.h>
#include <absmath/HeightMap.h>
#include <absmath/MathUtils.h>
#include <absmath/Matrix4.h>
#include <absmath/Plane.h>
#include <absmath/Point.h>
#include <absmath/Quaternion.h>
#include <absmath/Ray.h>
#include <absmath/Shape.h>
#include <absmath/Transformation.h>
#include <absmath/Vector3.h>
#include <absmath/VectorMath.h>
#include <base64.h>
#include <multi_index/hashed_index.hpp>
#include <multi_index/member.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index_container.hpp>
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
