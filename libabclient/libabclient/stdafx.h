// stdafx.h: Includedatei f�r Standardsystem-Includedateien
// oder h�ufig verwendete projektspezifische Includedateien,
// die nur in unregelm��igen Abst�nden ge�ndert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <cassert>
#include <ctype.h>

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#include <sys/timeb.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#define USE_STANDALONE_ASIO
#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)

#define AB_UNUSED(P) (void)(P)
