#pragma once

#include <map>
#include <string>
#include <memory>
#include "Skill.h"

namespace Game {

class SkillManager
{
public:
    SkillManager() = default;
    ~SkillManager() = default;

    std::map<uint32_t, std::string> skills_;

    std::shared_ptr<Skill> GetSkill(uint32_t id);
public:
    static SkillManager Instance;
};

}
