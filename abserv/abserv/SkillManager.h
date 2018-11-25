#pragma once

#include "Skill.h"

namespace Game {

class SkillManager
{
private:
    std::unordered_map<uint32_t, AB::Entities::Skill> skillCache_;
public:
    SkillManager();
    ~SkillManager() = default;

    std::shared_ptr<Skill> Get(uint32_t index);
};

}
