#pragma once

#include "Skill.h"
#include <array>
#include <memory>

namespace Game {

class SkillBar
{
private:
    std::array<std::shared_ptr<Skill>, PLAYER_MAX_SKILLS> spells_;
public:
    SkillBar();
    ~SkillBar();

    Skill* operator[](uint32_t index)
    {
        return spells_[index].get();
    }
};

}
