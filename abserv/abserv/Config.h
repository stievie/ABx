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
//#   define DEBUG_SQL
#   define DEBUG_GAME
#   ifdef DEBUG_GAME
#       define DEBUG_NAVIGATION
#   endif
#   define DEGUB_ENCRYPTION
#else
#endif

// Database drivers
#define USE_MYSQL
#define USE_PGSQL

// Name of config file
#define CONFIG_FILE "config.lua"
// Servers DH keys
#define DH_KEYS_FILE "server.key"

#define SCHEDULER_MINTICKS 10
// ms = 20 network ticks/second. Update game state each 50ms (= 20 Updates/sec)
#define NETWORK_TICK 50
// Maximum players per game. If reached a new instance of the same game is started.
#define GAME_MAX_PLAYER 100
#define PLAYER_MAX_SKILLS 8

