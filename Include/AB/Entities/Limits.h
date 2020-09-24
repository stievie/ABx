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

#include <limits>
#include <AB/CommonConfig.h>

namespace AB {
namespace Entities {
namespace Limits {
// General
static constexpr int MAX_UUID = 36;
static constexpr int MAX_FILENAME = 260;
// Service
static constexpr int MAX_SERVICE_NAME = 64;
static constexpr int MAX_SERVICE_HOST = 64;
static constexpr int MAX_SERVICE_IP = 64;
static constexpr int MAX_SERVICE_LOCATION = 10;
static constexpr int MAX_SERVICES = 64;         // Max number of services in ServiceList

static constexpr int MAX_MUSIC = 65536;
static constexpr int MAX_MUSIC_MAPS = 5000;

// Account
static constexpr int MAX_ACCOUNT_NAME = ACCOUNT_NAME_MAX;
static constexpr int MAX_ACCOUNT_PASS = PASSWORD_LENGTH_MAX;
static constexpr int MAX_ACCOUNT_EMAIL = EMAIL_LENGTH_MAX;
static constexpr int MAX_ACCOUNT_CHARACTERS = 50;   // Max number of characters
static constexpr int MAX_ACCOUNTS = std::numeric_limits<int>::max();
// AccountKey
static constexpr int MAX_ACCOUNTKEY_DESCRIPTION = 255;
static constexpr int MAX_ACCOUNTKEYS = std::numeric_limits<int>::max();

// Character
static constexpr int MAX_CHARACTER_NAME = 32;
static constexpr int MAX_CHARACTER_PROF = 2;
static constexpr int MAX_CHARACTER_SKILLTEMPLATE = 64;
static constexpr int MAX_CHARACTERS = std::numeric_limits<int>::max();
static constexpr int MAX_DEATH_STATS = 64;                         // Max size of stats BLOB

// Map
static constexpr int MAX_MAP_NAME = 50;
static constexpr int MAX_GAMES = 500;
// Game instance
static constexpr int MAX_GAME_INSTANCE_NAME = 500;
static constexpr int MAX_GAME_INSTANCES = 65536;

// Guild
static constexpr int MAX_GUILD_NAME = 32;
static constexpr int MAX_GUILD_TAG = 4;
static constexpr int MAX_GUILD_MEMBERS = 100;

// Ban
static constexpr int MAX_BAN_COMMENT = 255;

// Version
static constexpr int MAX_VERSION_NAME = 20;
static constexpr int MAX_VERSIONS = 256;

static constexpr int MAX_RESERVED_NAME = 40;

// Friendlist
static constexpr int MAX_FRIENDS = 100;
static constexpr int MAX_FRIENDED_ME = std::numeric_limits<int>::max();

// Quest
static constexpr int MAX_QUESTS = 1024;
static constexpr int MAX_QUESTNAME = 256;
static constexpr int MAX_QUESTDESCR = 4096;
static constexpr int MAX_QUESTPROGRESS = 1024;
static constexpr int MAX_PLAYERQUESTS = 64;
static constexpr int MAX_QUEST_REWARDITEMS = 4;

// Party
static constexpr int MAX_PARTY_MEMBERS = 12;

// Mail
static constexpr int MAX_MAIL_SUBJECT = 60;
static constexpr int MAX_MAIL_MESSAGE = 255;
static constexpr int MAX_MAIL_COUNT = 100;

// Professions
static constexpr int MAX_PROFESSION_NAME = 32;
static constexpr int MAX_PROFESSION_ABBR = 2;
static constexpr int MAX_PROFESSION_ATTRIBUTES = 5;
static constexpr int MAX_PROFESSIONS = 24;                            // Max professions in game
// Attribute
static constexpr int MAX_ATTRIBUTE_NAME = 32;
static constexpr int MAX_ATTRIBUTES = 64;
// Skill
static constexpr int MAX_SKILL_NAME = 64;
static constexpr int MAX_SKILL_DESCRIPTION = 255;
static constexpr int MAX_SKILL_SHORT_DESCRIPTION = 255;
static constexpr int MAX_SKILLS = 4096;                            // Max skills in game
// Effect
static constexpr int MAX_EFFECT_NAME = 64;
static constexpr int MAX_EFFECTS = 4096;                            // Max effects in game
// Item
static constexpr int MAX_ITEM_NAME = 64;
static constexpr int MAX_ITEMS = 65536;
static constexpr int MAX_ITEM_STATS = 1024;                         // Max size of stats BLOB

}
}
}
