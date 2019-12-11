#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4702 4127)
#   include <lua.hpp>
#   include <kaguya/kaguya.hpp>
PRAGMA_WARNING_POP
