#include "stdafx.h"
#include "SkillBar.h"

namespace Game {

void SkillBar::RegisterLua(kaguya::State& state)
{
    state["SkillBar"].setClass(kaguya::UserdataMetatable<SkillBar>()
        .addFunction("GetSkill", &SkillBar::GetSkill)
        .addFunction("GetCurrentSkill", &SkillBar::GetCurrentSkill)
    );
}

void SkillBar::UseSkill(int index, std::shared_ptr<Creature> target)
{
    Skill* s = skills_[index].get();
    if (s->IsChangingState())
        skillQueue_.push({ index, target });
    else
        s->StartUse(owner_, target.get());
}

Skill* SkillBar::GetCurrentSkill()
{
    if (currentSkillIndex_ > -1 && currentSkillIndex_ < PLAYER_MAX_SKILLS)
        return skills_[currentSkillIndex_].get();
    return nullptr;
}

void SkillBar::Update(uint32_t timeElapsed)
{
    int newSkill = -1;
    for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
    {
        if (skills_[i])
        {
            skills_[i]->Update(timeElapsed);
            if (skills_[i]->IsUsing())
                newSkill = i;
        }
    }
    if (newSkill == -1 && !skillQueue_.empty())
    {
        std::pair<uint32_t, std::weak_ptr<Creature>> sk = skillQueue_.front();
        skillQueue_.pop();
        std::shared_ptr<Skill> skill = GetSkill(sk.first);
        if (skill && skill->StartUse(owner_, sk.second.lock().get()))
            newSkill = sk.first;
    }
    currentSkillIndex_ = newSkill;
}

// https://wiki.guildwars.com/wiki/Skill_template_format
// https://wiki.guildwars.com/wiki/Equipment_template_format
std::string SkillBar::Encode()
{
    return std::string();
}

bool SkillBar::Decode(const std::string& str)
{
    return false;
}

std::shared_ptr<Skill> SkillBar::GetSkill(uint32_t index)
{
    if (index < PLAYER_MAX_SKILLS)
        return skills_[index];
    return std::shared_ptr<Skill>();
}

AttributeValue* SkillBar::GetAttribute(uint32_t index)
{
    if (index < PLAYER_MAX_ATTRIBUTES)
        return &attributes_[index];
    return nullptr;
}

}
