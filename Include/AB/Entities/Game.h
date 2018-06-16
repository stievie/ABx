#pragma once

#include <AB/Entities/Entity.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

constexpr auto KEY_GAMES = "game_maps";

enum GameType : uint8_t
{
    GameTypeUnknown = 0,
    GameTypeOutpost = 1,
    GameTypePvPCombat,
    GameTypeExploreable,
    GameTypeMission,
};

struct Game : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GAMES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_MAP_NAME);
        s.text1b(directory, Limits::MAX_FILENAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.value1b(type);
        s.value1b(landing);
        s.value1b(partySize);
    }

    /// The name of the game
    std::string name;
    /// Directory with game data
    std::string directory;
    /// Script file
    std::string script;
    GameType type = GameTypeUnknown;
    bool landing = false;
    uint8_t partySize = 0;
};

}
}
