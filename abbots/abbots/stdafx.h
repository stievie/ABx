#if defined(_MSC_VER)
#pragma once
#endif

#include <sa/Compiler.h>

#if defined(_MSC_VER)
#define _WIN32_WINNT 0x0601
#endif

PRAGMA_WARNING_DISABLE_MSVC(4307)

#include <stdio.h>
#include <iostream>

#include <stdint.h>
#include <sa/Assert.h>

#if !defined(ASIO_STANDALONE)
#define ASIO_STANDALONE
#endif
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4592)
#   include <asio.hpp>
PRAGMA_WARNING_POP

#include <abscommon/Utils.h>
#include <abscommon/Logger.h>
