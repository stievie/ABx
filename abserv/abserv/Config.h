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
#       define DEBUG_PROTOCOL
#   endif
#   define DEGUB_ENCRYPTION
#else
#endif

#define _PROFILING

#define HAVE_DIRECTX_MATH

// Name of config file
#define CONFIG_FILE "config.lua"
// Servers DH keys
#define DH_KEYS_FILE "server.key"

#define SCHEDULER_MINTICKS 10

// ms = 20 network ticks/second. Update game state each 50ms (= 20 Updates/sec)
#define NETWORK_TICK 50
// Update game state each 33ms (= 30 Updates/sec)
//#define NETWORK_TICK 33

// Maximum players per game. If reached a new instance of the same game is started.
#define GAME_MAX_PLAYER 100
// Maximum connections to this server. A single machine can maybe handle up to 3000 concurrent connections.
// https://www.gamedev.net/forums/topic/319003-mmorpg-and-the-ol-udp-vs-tcp/?do=findComment&comment=3052256
#define SERVER_MAX_CONNECTIONS 3000
#define PLAYER_MAX_SKILLS 8

// Clean cache every 10min
#define CLEAN_CACHE_MS (1000 * 60 * 10)
// Clean games all 10 sec
#define CLEAN_GAMES_MS (1000 * 10)
// Remove inactive players every 30sec
#define CLEAN_PLAYERS_MS (1000 * 30)
// Remove player from PlayerManager after this time
#define PLAYER_INACTIVE_TIME_KICK (10 * 1000)

#define ROLL_MIN 2
#define ROLL_MAX 100
