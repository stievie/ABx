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
#include <AB/Entities/Game.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_GAMEINSTANCES = "game_instances";

struct GameInstance : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GAMEINSTANCES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(gameUuid, Limits::MAX_UUID);
        s.text1b(serverUuid, Limits::MAX_UUID);
        s.value8b(startTime);
        s.value2b(number);
    }

    std::string gameUuid = EMPTY_GUID;
    /// Server running this instance
    std::string serverUuid = EMPTY_GUID;
    int64_t startTime = 0;
    /// If there are more instances of the same game on the same server this is the number, e.g. District.
    uint16_t number = 0;
};

}
}
