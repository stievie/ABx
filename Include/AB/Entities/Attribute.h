#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_ATTRIBUTES = "game_attributes";

struct Attribute : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ATTRIBUTES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(professionUuid, Limits::MAX_UUID);
        s.text1b(name, Limits::MAX_ATTRIBUTE_NAME);
        s.value1b(isPrimary);
    }

    uint32_t index = INVALID_INDEX;
    std::string professionUuid = EMPTY_GUID;
    std::string name;
    bool isPrimary = false;
};

}
}
