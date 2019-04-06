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

static constexpr auto KEY_ITEMCHANCELIST = "game_item_chances_list";

struct ItemChanceList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ITEMCHANCELIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(items, Limits::MAX_ITEMS, [&s](std::pair<std::string, float>& c)
        {
            s.text1b(c.first, Limits::MAX_UUID);
            s.value4b(c.second);
        });
    }

    std::vector<std::pair<std::string, float>> items;
};

}
}
