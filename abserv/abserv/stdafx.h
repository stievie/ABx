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

#include "BoundingBox.h"
#include "CollisionComp.h"
#include "HeightMap.h"
#include "Matrix4.h"
#include "Point.h"
#include "Quaternion.h"
#include "Transformation.h"
#include "Vector3.h"
#include "VectorMath.h"

#include "Agent.h"
#include "AiAgent.h"
#include "Actor.h"
#include "Application.h"
#include "Attributes.h"
#include "Damage.h"
#include "DataClient.h"
#include "Game.h"
#include "GameObject.h"
#include "Logger.h"
#include "Map.h"
#include "MathUtils.h"
#include "Mechanic.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Player.h"
#include "PlayerManager.h"
#include "Profiler.h"
#include "PropStream.h"
#include "Script.h"
#include "StringUtils.h"
#include "Subsystems.h"
#include "Utils.h"
#include "UuidUtils.h"
#include "Variant.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/GuildMembers.h>
#include <AB/Entities/Mail.h>
#include <AB/Entities/MailList.h>
#include <AB/Packets/ClientPackets.h>
#include <AB/ProtocolCodes.h>
#include <base64.h>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index/member.hpp>
#include <sa/CallableTable.h>
#include <sa/CircularQueue.h>
#include <sa/Events.h>
#include <sa/IdGenerator.h>
#include <sa/Iteration.h>
#include <sa/StringHash.h>
#include <sa/StrongType.h>
#include <sa/TypeName.h>
#include <sa/WeightedSelector.h>
#include <uuid.h>
