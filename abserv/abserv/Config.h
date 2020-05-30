/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

// Name of config file
#define CONFIG_FILE "abserv.lua"

#if !defined(WIN_SERVICE)
    // Compiled with Debug::SceneViewer
#   if !defined(SCENE_VIEWER)
//#      define SCENE_VIEWER
#   endif
#else
#   undef SCENE_VIEWER
#endif
#define FREEGLUT_STATIC
#define HAVE_CONFIG_H
#define GLEW_STATIC

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
// Clean assets cache every 10min (ms)
#define CACHE_CLEAN_INTERVAL (1000 * 60 * 10)
// How long keep unused assets in cache (ms)
#define CACHE_KEEP_UNUSED_ASSETS (1000 * 60 * 30)
// Clean games all 10 sec
#define CLEAN_GAMES_MS (1000 * 10)
// Remove inactive players every 10sec
#define CLEAN_PLAYERS_MS (1000 * 10)
// Clean empty chats all minutes
#define CLEAN_CHATS_MS (1000 * 60)
// Check to terminate if the are no players left
#define CHECK_AUTOTERMINATE_MS (1000 * 60)
// Server must be idle for 10min to automatically terminate
#define CHECK_AUTOTERMINATE_IDLE_MS (1000 * 60 * 10)
// Remove player from PlayerManager after this time. Inactive means not Ping.
#define PLAYER_INACTIVE_TIME_KICK (1000 * 15)
// Game is inactive and will be stopped when it didn't have a player for this time.
#define GAME_INACTIVE_TIME (1000 * 60)
// Time after a party is teleported back to the outpost after it was defeated/resigned in ms
#define PARTY_TELEPORT_BACK_TIME (2000u)
#define AI_SERVER_UPDATE_INTERVAL (1000)
#define FILEWATCHER_INTERVAL (100)

#define ROLL_MIN 2
#define ROLL_MAX 100
