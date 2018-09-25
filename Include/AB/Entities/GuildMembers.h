#pragma once

#include <AB/Entities/Entity.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <bitsery/traits/vector.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_GUILDMEMBERS = "guild_mebmers";

// 0 = Unknown, 1 = Guest, 2 = Invited, 3 = Member, 4 = Officer, 5 = Leader
enum GuildRole : uint8_t
{
    GuildRoleUnknown = 0,
    GuildRoleGuest,
    GuildRoleInvited,
    GuildRoleMember,
    GuildRoleOfficer,
    GuildRoleLeader
};

struct GuildMember
{
    std::string accountUuid;
    std::string inviteName;
    GuildRole role = GuildRoleUnknown;
    /// Time invited
    int64_t invited = 0;
    /// Time joined
    int64_t joined = 0;
    /// Time membership expires or 0 does not expire
    int64_t expires = 0;
};

/// List of Guild members. UUID is the Guild UUID. To add members to a Guild, add
/// to this list and save it.
struct GuildMembers : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GUILDMEMBERS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(members, Limits::MAX_GUILD_MEMBERS, [&s](GuildMember& c)
        {
            s.text1b(c.accountUuid, Limits::MAX_UUID);
            s.text1b(c.inviteName, Limits::MAX_CHARACTER_NAME);
            s.value1b(c.role);
            s.value8b(c.invited);
            s.value8b(c.joined);
            s.value8b(c.expires);
        });
    }

    std::vector<GuildMember> members;
};

}
}
