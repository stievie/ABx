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
#include <cassert>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>

#include <AB/CommonConfig.h>
#include <abscommon/DebugConfig.h>

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif
PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
#   include <asio.hpp>
PRAGMA_WARNING_POP

#include <uuid.h>

#define WRITE_MINIBUMP
#define AB_UNUSED(P) (void)(P)

#include <abscommon/Utils.h>
#include <abscommon/Logger.h>

// Maximum connections to this server. A single machine can maybe handle up to 3000 concurrent connections.
// https://www.gamedev.net/forums/topic/319003-mmorpg-and-the-ol-udp-vs-tcp/?do=findComment&comment=3052256
#define SERVER_MAX_CONNECTIONS 3000

// When a user deletes a character, the characters name is reserved for 1 week
static constexpr int NAME_RESERVATION_EXPIRES_MS = 1000 * 60 * 60 * 24 * 7;
