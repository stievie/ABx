#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_PROFESSIONS = "game_professions";

struct Profession : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_PROFESSIONS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_PROFESSION_NAME);
        s.text1b(abbr, Limits::MAX_PROFESSION_ABBR);
    }

    uint32_t index = 0;
    std::string name;
    std::string abbr;
};

}
}
