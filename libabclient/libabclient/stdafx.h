#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

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
#include "Windows.h"
#endif

#define USE_STANDALONE_ASIO

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4592)
#include <asio.hpp>
PRAGMA_WARNING_POP

#include <base64.h>

#define AB_UNUSED(P) (void)(P)
