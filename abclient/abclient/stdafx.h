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
#include <sa/PragmaWarning.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#undef MessageBox                       // Redefines Urho3D::MessageBox
#undef GetMessage
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
