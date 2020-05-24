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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#if defined(_MSC_VER)
#pragma once
#endif

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#include "targetver.h"
#include <sa/Compiler.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <CleanupNs.h>
#endif // _WIN32

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdint.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <cassert>
#include <string>
#include <sstream>

#include "Defines.h"
#include "Config.h"

#define USE_STANDALONE_ASIO

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4592)
#include <asio.hpp>
PRAGMA_WARNING_POP

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4100 4305 4800 4244)
PRAGMA_WARNING_DISABLE_GCC("-Wstrict-aliasing")
PRAGMA_WARNING_DISABLE_GCC("-Wreorder")
#include <Urho3D/Urho3DAll.h>
PRAGMA_WARNING_POP

#include "InternalEvents.h"
#include "ShortcutEvents.h"
#include "ServerEvents.h"
