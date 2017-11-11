#include "stdafx.h"
#include "SkillBar.h"

namespace Game {

void SkillBar::Update(uint32_t timeElapsed)
{
    for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
    {
        if (skills_[i])
            skills_[i]->Update(timeElapsed);
    }
}

std::string SkillBar::Encode()
{
    return std::string();
}

bool SkillBar::Decode(const std::string& str)
{
    return false;
}

}
