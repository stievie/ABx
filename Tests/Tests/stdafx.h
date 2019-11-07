#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4307)
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschließen
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdint.h>
#include <initializer_list>

#include "MathConfig.h"
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4702 4127)
#endif
#include <lua.hpp>
#include <kaguya/kaguya.hpp>
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif
