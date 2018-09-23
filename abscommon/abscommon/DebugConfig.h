#pragma once

// Used by several projects

#if defined(_DEBUG)
//#   define DEBUG_DISPATTCHER
//#   define DEBUG_SCHEDULER
#   define DEBUG_NET
//#   define DEBUG_SQL
//#   define DEBUG_MATH
#   define DEBUG_GAME
#   ifdef DEBUG_GAME
#       define DEBUG_NAVIGATION
#       define DEBUG_PROTOCOL
//#       define DEBUG_OCTREE
//#       define DEBUG_COLLISION
#   endif
#   define DEGUB_ENCRYPTION
#else
#endif

#define _PROFILING
