// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"

#include <stdio.h>
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#define USE_STANDALONE_ASIO
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4592)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#define AB_UNUSED(P) (void)(P)
