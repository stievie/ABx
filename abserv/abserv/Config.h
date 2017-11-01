#pragma once

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#if defined(_DEBUG)
//#   define DEBUG_DISPATTCHER
//#   define DEBUG_SCHEDULER
#   define DEBUG_NET
//#   define DEBUG_MATH
#define DEBUG_SQL
#else
#endif

#define USE_MYSQL

#define CONFIG_FILE "config.lua"
