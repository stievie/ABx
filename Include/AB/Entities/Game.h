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

#include <AB/Entities/Entity.h>

namespace AB {
namespace Entities {

enum GameType : uint8_t
{
    GameTypeUnknown = 0,
    GameTypeOutpost = 1,
    GameTypeTown,
    GameTypeGuildHall,
    // Bellow this games are exclusive and always created new
    GameTypePvPCombat,
    GameTypeExploreable,
    GameTypeMission,
};

inline bool IsOutpost(GameType type)
{
    return type >= GameTypeOutpost && type <= GameTypeGuildHall;
}

enum GameModeFlags : uint32_t
{
    GameModeFlagNone = 0,
};

enum GameMode : uint32_t
{
    GameModeUnknown = 0,
};

struct Game : Entity
{
    MAKE_ENTITY(Game)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_MAP_NAME);
        s.text1b(directory, Limits::MAX_FILENAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(queueMapUuid, Limits::MAX_UUID);
        s.value1b(type);
        s.value4b(mode);
        s.value1b(landing);
        s.value1b(partySize);
        s.value1b(partyCount);
        s.value1b(randomParty);
        s.value4b(mapCoordX);
        s.value4b(mapCoordY);
        s.value1b(defaultLevel);
    }

    /// The name of the game
    std::string name;
    /// Directory with game data
    std::string directory;
    /// Script file
    std::string script;
    std::string queueMapUuid;
    GameType type = GameTypeUnknown;
    GameMode mode = GameModeUnknown;
    bool landing = false;
    uint8_t partySize = 0;
    uint8_t partyCount = 0;
    bool randomParty = false;
    int32_t mapCoordX = 0;
    int32_t mapCoordY = 0;
    int8_t defaultLevel = 1;
};

}
}
