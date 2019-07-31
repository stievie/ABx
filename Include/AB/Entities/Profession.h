#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>
#include <vector>

namespace AB {
namespace Entities {

enum ProfessionIndex : uint32_t
{
    ProfessionIndexNone = 0,
    ProfessionIndexWarrior = 1,
    ProfessionIndexRanger = 2,
    ProfessionIndexMonk = 3,
    ProfessionIndexNecromancer = 4,
    ProfessionIndexMesmer = 5,
    ProfessionIndexElementarist = 6
};

struct AttriInfo
{
    std::string uuid;
    uint32_t index{ 0 };
    bool primary{ false };
};

static constexpr auto KEY_PROFESSIONS = "game_professions";

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
        s.value4b(modelIndexFemale);
        s.value4b(modelIndexMale);
        s.value4b(attributeCount);
        s.container(attributes, Limits::MAX_PROFESSION_ATTRIBUTES, [&s](AttriInfo& a)
        {
            s.text1b(a.uuid, Limits::MAX_UUID);
            s.value4b(a.index);
            s.value1b(a.primary);
        });
    }

    uint32_t index = INVALID_INDEX;
    std::string name;
    std::string abbr;
    uint32_t modelIndexFemale = 0;
    uint32_t modelIndexMale = 0;
    uint32_t attributeCount = 0;
    std::vector<AttriInfo> attributes;
};

}
}
