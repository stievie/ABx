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

constexpr auto KEY_GUILDMEMBERS = "guild_mebmers";

enum GuildRole : uint8_t
{
    GuildRoleUnknown = 0,
    GuildRoleGuest,
    GuildRoleMember,
    GuildRoleOfficer,
    GuildRoleLeader
}

struct GuildMember : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GUILDMEMBERS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(account_uuid, Limits::MAX_UUID);
        s.text1b(guild_uuid, Limits::MAX_UUID);
        s.value1b(role);
    }

    std::string account_uuid;
    std::string guild_uuid;
    GuildRole role;
};

}
}
