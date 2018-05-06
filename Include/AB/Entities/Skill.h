#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_SKILLS = "game_skills";

struct Skill : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_SKILLS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_SKILL_NAME);
        s.text1b(attributeUuid, Limits::MAX_UUID);
        s.value4b(type);
        s.value1b(isElite);
        s.text1b(description, Limits::MAX_SKILL_DESCRIPTION);
        s.text1b(shortDescription, Limits::MAX_SKILL_SHORT_DESCRIPTION);
        s.text1b(icon, Limits::MAX_FILENAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.value1b(isLocked);
    }

    uint32_t index = 0;
    std::string name;
    std::string attributeUuid;
    uint32_t type = 0;
    bool isElite = false;
    std::string description;
    std::string shortDescription;
    std::string icon;
    std::string script;
    bool isLocked = false;
};

}
}
