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

enum FriendRelation : uint8_t
{
    FriendRelationUnknown = 0,
    FriendRelationFriend = 1,
    FriendRelationIgnore = 2
};

struct Friend
{
    /// Friend account UUID
    std::string friendUuid;
    /// Character or nick name of the friend
    std::string friendName;
    FriendRelation relation{ FriendRelationUnknown };
    timestamp_t creation;
};

/// Friendlist. UUID is Account UUID
struct FriendList : Entity
{
    MAKE_ENTITY(FriendList)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(friends, Limits::MAX_FRIENDS, [&s](Friend& c)
        {
            s.text1b(c.friendUuid, Limits::MAX_UUID);
            s.text1b(c.friendName, Limits::MAX_CHARACTER_NAME);
            s.value1b(c.relation);
            s.value8b(c.creation);
        });
    }

    std::vector<Friend> friends;
};

}
}
