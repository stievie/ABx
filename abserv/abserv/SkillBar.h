#pragma once

#include "Skill.h"
#include <array>
#include <queue>
#include <AB/Entities/Profession.h>
#include <AB/CommonConfig.h>
#include <AB/TemplEncoder.h>

namespace Game {

class Actor;

typedef std::array<std::shared_ptr<Skill>, PLAYER_MAX_SKILLS> SkillsArray;

class SkillBar
{
private:
    SkillsArray skills_;
    AB::Attributes attributes_;
    Actor* owner_;
    int currentSkillIndex_;
    int _LuaAddSkill(uint32_t skillIndex);
    std::vector<uint32_t> _LuaGetSkillsWithEffect(uint32_t effect) const { return GetSkillsWithEffect(static_cast<SkillEffect>(effect)); }
    std::vector<uint32_t> _LuaGetSkillsWithTarget(uint32_t target) const { return GetSkillsWithTarget(static_cast<SkillTarget>(target)); }
public:
    static void RegisterLua(kaguya::State& state);

    SkillBar(Actor* owner) :
        owner_(owner),
        currentSkillIndex_(-1)
    {
    };
    ~SkillBar() = default;

    /// 0 Based
    AB::GameProtocol::SkillError UseSkill(int index, std::shared_ptr<Actor> target);
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
    AB::AttributeValue* GetAttribute(uint32_t index);
    const AB::Attributes& GetAttributes() const { return attributes_; }
    const SkillsArray& GetArray() const { return skills_; }
    std::vector<uint32_t> GetSkillsWithEffect(SkillEffect effect) const;
    std::vector<uint32_t> GetSkillsWithTarget(SkillTarget target) const;

    Skill* operator[](uint32_t index)
    {
        return skills_[index].get();
    }

    AB::Entities::Profession prof1_;
    AB::Entities::Profession prof2_;
};

}
