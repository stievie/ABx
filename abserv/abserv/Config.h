#pragma once

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#define HAVE_DIRECTX_MATH

// Name of config file
#define CONFIG_FILE "abserv.lua"

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

// Update server load every second
#define UPDATE_SERVER_LOAD_MS (1000)
// Clean assets cache every 10min
#define CLEAN_CACHE_MS (1000 * 60 * 10)
// Clean games all 10 sec
#define CLEAN_GAMES_MS (1000 * 10)
// Remove inactive players every 10sec
#define CLEAN_PLAYERS_MS (1000 * 10)
// Clean empty chats all minutes
#define CLEAN_CHATS_MS (1000 * 60)
// Check to terminate if the are no players left
#define CHECK_AUTOTERMINATE_MS (1000 * 60)
// Remove player from PlayerManager after this time. Inactive means not Ping.
#define PLAYER_INACTIVE_TIME_KICK (1000 * 15)
// Game is inactive and will be stopped when it didnt have a player for this time.
#define GAME_INACTIVE_TIME (1000 * 60)

#define ROLL_MIN 2
#define ROLL_MAX 100
