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

struct GameInstance : Entity
{
    static constexpr const char* KEY()
    {
        return "instances";
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(gameUuid, Limits::MAX_UUID);
        s.text1b(serverUuid, Limits::MAX_UUID);
        s.text1b(name, Limits::MAX_GAME_INSTANCE_NAME);
        s.text1b(recording, Limits::MAX_FILENAME);
        s.value8b(startTime);
        s.value8b(stopTime);
        s.value2b(number);
        s.value1b(running);
    }

    std::string gameUuid = EMPTY_GUID;
    /// Server running this instance
    std::string serverUuid = EMPTY_GUID;
    std::string name;
    /// Recording filename
    std::string recording;
    timestamp_t startTime{ 0 };
    timestamp_t stopTime{ 0 };
    /// If there are more instances of the same game on the same server this is the number, e.g. District.
    uint16_t number = 0;
    bool running{ false };
};

}
}
