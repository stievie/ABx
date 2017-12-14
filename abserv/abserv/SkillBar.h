#pragma once

#include "Skill.h"
#include <array>

namespace Game {

class SkillBar
{
private:
    std::array<std::shared_ptr<Skill>, PLAYER_MAX_SKILLS> skills_;
public:
    SkillBar() = default;
    ~SkillBar() = default;

    void Update(uint32_t timeElapsed);
    std::string Encode();
    bool Decode(const std::string& str);

    Skill* operator[](uint32_t index)
    {
        return skills_[index].get();
    }
};

}
