#pragma once

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#if defined(_DEBUG)
#define DEBUG_DISPATTCHER
#define DEBUG_SCHEDULER
#else
#endif