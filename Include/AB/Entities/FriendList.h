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

constexpr auto KEY_FRIENDLIST = "friend_list";

enum FriendRelation : uint8_t
{
    FriendRelationFriend = 0,
    FriendRelationIgnore = 1
};

struct FriendList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_FRIENDLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(account_uuid, Limits::MAX_UUID);
        s.text1b(friend_uuid, Limits::MAX_UUID);
        s.value1b(relation);
    }

    std::string account_uuid;
    std::string friend_uuid;
    FriendRelation relation;
};

}
}
