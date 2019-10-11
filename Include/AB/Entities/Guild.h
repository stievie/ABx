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

static constexpr auto KEY_GUILD = "guilds";

struct Guild : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GUILD;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_GUILD_NAME);
        s.text1b(tag, Limits::MAX_GUILD_TAG);
        s.text1b(creatorAccountUuid, Limits::MAX_UUID);
        s.text1b(creatorName, Limits::MAX_CHARACTER_NAME);
        s.text1b(creatorPlayerUuid, Limits::MAX_UUID);
        s.value8b(creation);
        s.text1b(guildHall, Limits::MAX_UUID);
        s.text1b(guildHallInstanceUuid, Limits::MAX_UUID);
        s.text1b(guildHallServerUuid, Limits::MAX_UUID);
    }

    std::string name;
    std::string tag;
    std::string creatorAccountUuid = EMPTY_GUID;
    std::string creatorName;
    std::string creatorPlayerUuid = EMPTY_GUID;
    timestamp_t creation;
    std::string guildHall = EMPTY_GUID;
    std::string guildHallInstanceUuid = EMPTY_GUID;
    std::string guildHallServerUuid = EMPTY_GUID;
};

}
}
