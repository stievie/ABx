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

typedef std::array<AttributeValue, PLAYER_MAX_ATTRIBUTES> Attributes;
typedef std::array<std::shared_ptr<Skill>, PLAYER_MAX_SKILLS> SkillsArray;

class SkillBar
{
private:
    SkillsArray skills_;
    Attributes attributes_;
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

    /// 0 Based
    bool UseSkill(int index, std::shared_ptr<Actor> target);
    Skill* GetCurrentSkill() const;
    void Update(uint32_t timeElapsed);
    std::string Encode();
    bool Load(const std::string& str, bool locked);

    std::shared_ptr<Skill> GetSkill(uint32_t index);
    void SetSkill(int index, std::shared_ptr<Skill> skill)
    {
        if (index < 0 || index >= PLAYER_MAX_SKILLS)
            return;
        skills_[index] = skill;
    }
    AttributeValue* GetAttribute(uint32_t index);
    const Attributes& GetAttributes() const { return attributes_; }
    const SkillsArray& GetArray() const { return skills_; }

    Skill* operator[](uint32_t index)
    {
        return skills_[index].get();
    }

    AB::Entities::Profession prof1_;
    AB::Entities::Profession prof2_;
};

}
