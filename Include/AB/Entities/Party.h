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

static constexpr auto KEY_GAMEPARTIES = "game_parties";

struct Party : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_GAMEPARTIES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(members, Limits::MAX_PARTY_MEMBERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
        s.container(npcs, Limits::MAX_PARTY_MEMBERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_FILENAME);
        });
    }

    // Player UUIDs
    std::vector<std::string> members;
    // NPC scripts
    std::vector<std::string> npcs;
};

}
}
