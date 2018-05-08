#pragma once

#include "Skill.h"
#include <array>
#include <queue>

namespace Game {

class Creature;

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
    Creature* owner_;
    std::queue<std::pair<int, std::weak_ptr<Creature>>> skillQueue_;
    int currentSkillIndex_;
public:
    static void RegisterLua(kaguya::State& state);

    SkillBar(Creature* owner) :
        owner_(owner),
        currentSkillIndex_(-1)
    {
    };
    ~SkillBar() = default;

    void UseSkill(int index, std::shared_ptr<Creature> target);
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
};

}
