#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

PRAGMA_WARNING_DISABLE_MSVC(4307)

#include <stdio.h>

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#define WRITE_MINIBUMP

#include <AB/CommonConfig.h>
#include "DebugConfig.h"

#define ASIO_STANDALONE

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4592)
#   include <asio.hpp>
PRAGMA_WARNING_POP

#include "Utils.h"
#include "Logger.h"
