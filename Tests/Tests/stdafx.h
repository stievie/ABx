#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

PRAGMA_WARNING_DISABLE_MSVC(4307)

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include <cmath>

#include <stdint.h>
#include <initializer_list>

#include "MathConfig.h"
PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4702 4127)
#   include <lua.hpp>
#   include <kaguya/kaguya.hpp>
PRAGMA_WARNING_POP
