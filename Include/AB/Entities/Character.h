#pragma once

#include "Entity.h"
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

struct Character : Entity
{
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(pvp);
        s.value8b(xp);
        s.value4b(skillPoints);
        s.text1b(lastMap, Limits::MAX_MAP_NAME);
    }

    /// PvP only character
    bool pvp = false;
    uint64_t xp = 0;
    uint32_t skillPoints = 0;
    std::string lastMap;
};

}
}
