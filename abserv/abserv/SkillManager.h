#pragma once

#include "Skill.h"

namespace Game {

class SkillManager
{
public:
    SkillManager() = default;
    ~SkillManager() = default;

    std::map<uint32_t, std::string> skills_;

    std::shared_ptr<Skill> Get(uint32_t id);
public:
    static SkillManager Instance;
};

}
