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
#include <bitsery/traits/vector.h>

namespace AB {
namespace Entities {

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
    std::string accountUuid = EMPTY_GUID;
    std::string inviteName;
    GuildRole role = GuildRoleUnknown;
    /// Time invited
    timestamp_t invited = 0;
    /// Time joined
    timestamp_t joined = 0;
    /// Time membership expires or 0 does not expire
    timestamp_t expires = 0;
};

/// List of Guild members. UUID is the Guild UUID. To add members to a Guild, add
/// to this list and save it.
struct GuildMembers : Entity
{
    MAKE_ENTITY(GuildMembers)
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
