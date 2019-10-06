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
#include <AB/Entities/FriendList.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

struct FriendAcc
{
    std::string accountUuid;
    FriendRelation relation{ FriendRelationUnknown };
};

/// All these have me in their friend list
struct FriendedMe : Entity
{
    static constexpr const char* KEY()
    {
        return "friended_me";
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(friends, Limits::MAX_FRIENDS, [&s](FriendAcc& c)
        {
            s.text1b(c.accountUuid, Limits::MAX_UUID);
            s.value1b(c.relation);
        });
    }

    std::vector<FriendAcc> friends;
};

}
}
