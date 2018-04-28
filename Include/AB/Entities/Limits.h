#pragma once

namespace AB {
namespace Entities {
namespace Limits {
// General
static constexpr int MAX_UUID = 36;
static constexpr int MAX_FILENAME = 260;
// Account
static constexpr int MAX_ACCOUNT_NAME = 32;
static constexpr int MAX_ACCOUNT_PASS = 61;
static constexpr int MAX_ACCOUNT_EMAIL = 60;
static constexpr int MAX_ACCOUNT_CHARACTERS = 50;
// AccountKey
static constexpr int MAX_ACCOUNTKEY_DESCRIPTION = 255;

// Character
static constexpr int MAX_CHARACTER_NAME = 20;
static constexpr int MAX_CHARACTER_PROF = 2;

// Map
static constexpr int MAX_MAP_NAME = 50;
static constexpr int MAX_GAMES = 500;

// Guild
static constexpr int MAX_GUILD_NAME = 32;
static constexpr int MAX_GUILD_TAG = 4;
static constexpr int MAX_GUILD_MEMBERS = 500;

// Ban
static constexpr int MAX_BAN_COMMENT = 255;

// Friendlist
static constexpr int MAX_FRIENDS = 1000;

}
}
}
