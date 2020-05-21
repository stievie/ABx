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

#include <iostream>
#include <map>
#include <string>

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
#include <base64.h>
#include <brigand/brigand.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/member.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index_container.hpp>

#define PROFILING

#include <abscommon/Logger.h>
#include <abscommon/UuidUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/Utils.h>
#include <abscommon/StringUtils.h>
#include <abdb/Database.h>

#define MAX_DATA_SIZE (1024 * 1024)
#define MAX_KEY_SIZE 256

#if !defined(USE_MYSQL) && !defined(USE_PGSQL) && !defined(USE_ODBC) && !defined(USE_SQLITE)
#error "Define at least one database driver"
#endif
