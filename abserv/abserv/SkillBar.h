#pragma once

#include "Skill.h"
#include <array>
#include <queue>
#include <AB/Entities/Profession.h>

namespace Game {

class Actor;

struct AttributeValue
{
    uint32_t index = 99;    // No attribute
    uint32_t value = 0;
};

class SkillBar
{
private:
    std::array<std::shared_ptr<Skill>, PLAYER_MAX_SKILLS> skills_;
    std::array<AttributeValue, PLAYER_MAX_ATTRIBUTES> attributes_;
    Actor* owner_;
    int currentSkillIndex_;
public:
    static void RegisterLua(kaguya::State& state);

    SkillBar(Actor* owner) :
        owner_(owner),
        currentSkillIndex_(-1)
    {
    };
    ~SkillBar() = default;

    bool UseSkill(int index, std::shared_ptr<Actor> target);
    Skill* GetCurrentSkill();
    void Update(uint32_t timeElapsed);
    std::string Encode();
    bool Decode(const std::string& str);

    std::shared_ptr<Skill> GetSkill(uint32_t index);
    AttributeValue* GetAttribute(uint32_t index);

    Skill* operator[](uint32_t index)
    {
        return skills_[index].get();
    }

    AB::Entities::Profession prof1_;
    AB::Entities::Profession prof2_;
};

}
