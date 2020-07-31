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

#if defined(_MSC_VER)
// Decorated name length exceeded
#pragma warning(disable: 4503 4307)
#endif

#include "targetver.h"
#include <sa/Compiler.h>

#include <stdio.h>
#include <stdint.h>

#include <memory>
#include <limits>
#include <unordered_map>

#include <AB/CommonConfig.h>
#include <abscommon/DebugConfig.h>

#if !defined(USE_STANDALONE_ASIO)
#define USE_STANDALONE_ASIO
#endif
#include "Servers.h"

#include <json.hpp>

#include <abscommon/DataClient.h>
#include <abscommon/FileUtils.h>
#include <abscommon/Logger.h>
#include <abscommon/MessageClient.h>
#include <abscommon/Profiler.h>
#include <abscommon/ServerApp.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/TimeUtils.h>
#include <abscommon/Utils.h>
#include <abscommon/UuidUtils.h>
#include <abscommon/Variant.h>
#include <abscommon/Xml.h>


#define PROFILING
