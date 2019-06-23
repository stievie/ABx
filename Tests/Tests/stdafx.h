#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Selten verwendete Komponenten aus Windows-Headern ausschlieﬂen
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdint.h>
#include <initializer_list>

#include "MathConfig.h"
