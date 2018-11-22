#include "stdafx.h"
#include "SkillBar.h"
#include <base64.h>

namespace Game {

void SkillBar::RegisterLua(kaguya::State& state)
{
    state["SkillBar"].setClass(kaguya::UserdataMetatable<SkillBar>()
        .addFunction("GetSkill", &SkillBar::GetSkill)
        .addFunction("GetCurrentSkill", &SkillBar::GetCurrentSkill)
        .addFunction("UseSkill", &SkillBar::UseSkill)
    );
}

bool SkillBar::UseSkill(int index, std::shared_ptr<Actor> target)
{
    Skill* s = skills_[index].get();
    if (s->IsChangingState())
    {
        // If changing state cancel old skill. Only one changing state skill at a time.
        Skill* oldSkill = GetCurrentSkill();
        if (oldSkill && oldSkill->IsUsing())
            oldSkill->CancelUse();
    }

    return s->StartUse(owner_, target.get());
}

Skill* SkillBar::GetCurrentSkill() const
{
    if (currentSkillIndex_ > -1 && currentSkillIndex_ < PLAYER_MAX_SKILLS)
    {
        Skill* skill = skills_[currentSkillIndex_].get();
        if (skill && skill->IsUsing())
            return skill;
    }
    return nullptr;
}

void SkillBar::Update(uint32_t timeElapsed)
{
    for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
    {
        if (skills_[i])
            skills_[i]->Update(timeElapsed);
    }
}

// https://wiki.guildwars.com/wiki/Skill_template_format
// https://wiki.guildwars.com/wiki/Equipment_template_format
// OgUUcRrg1MT6WOBqGIG/aKHXi+G
std::string SkillBar::Encode()
{
    // TODO:
    std::vector<uint8_t> buff;
    // Type, version
    buff.push_back((0xe << 4) | 0);

    return base64::encode(buff.data(), buff.size());
}

bool SkillBar::Decode(const std::string& str)
{
    // TODO:
    std::string s = base64::decode(str);
    if (s[0] != ((0xe << 4) | 0))
        return false;

    return true;
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
