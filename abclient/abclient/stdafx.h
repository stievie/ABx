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

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#undef MessageBox                       // Redefines Urho3D::MessageBox
#undef GetMessage

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdint.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <cassert>
#include <string>
#include <sstream>

#include "Config.h"

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>

#pragma warning( pop )
#include "NuklearUI.h"
#define NK_MEMSET std::memset