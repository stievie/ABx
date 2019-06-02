// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#include "targetver.h"

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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4100 4305 4800 4244)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
#include <Urho3D/Urho3DAll.h>
#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#endif
