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
    Actor& owner_;
    int currentSkillIndex_{ -1 };
    int _LuaAddSkill(uint32_t skillIndex);
    std::vector<uint32_t> _LuaGetSkillsWithEffect(uint32_t effect) const {
        return GetSkillsWithEffect(static_cast<SkillEffect>(effect));
    }
    std::vector<uint32_t> _LuaGetSkillsWithTarget(uint32_t target) const {
        return GetSkillsWithTarget(static_cast<SkillTarget>(target));
    }
    std::vector<uint32_t> _LuaGetSkillsWithEffectTarget(uint32_t effect, uint32_t target) const {
        return GetSkillsWithEffectTarget(static_cast<SkillEffect>(effect), static_cast<SkillTarget>(target));
    }
    Skill* _LuaGetSkill(uint32_t index);
    void SetAttributes(const AB::Attributes& attributes);
    void ResetAttributes();
    bool HaveAttribute(uint32_t index);
public:
    static void RegisterLua(kaguya::State& state);

    SkillBar() = delete;
    explicit SkillBar(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    SkillBar(const SkillBar&) = delete;
    SkillBar& operator=(const SkillBar&) = delete;
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
    void InitAttributes();
    /// Get an attribute
    /// @param index The index of the attribute, not the index in the array
    /// @return The attribute or nullptr
    const AB::AttributeValue* GetAttribute(uint32_t index) const;
    const AB::Attributes& GetAttributes() const { return attributes_; }
    bool SetAttributeValue(uint32_t index, uint32_t value);
    uint32_t GetAttributeValue(uint32_t index) const;
    const SkillsArray& GetArray() const { return skills_; }
    std::vector<uint32_t> GetSkillsWithEffect(SkillEffect effect, bool rechargedOnly = false) const;
    std::vector<uint32_t> GetSkillsWithTarget(SkillTarget target, bool rechargedOnly = false) const;
    std::vector<uint32_t> GetSkillsWithEffectTarget(SkillEffect effect, SkillTarget target, bool rechargedOnly = false) const;

    Skill* operator[](uint32_t index)
    {
        return skills_[index].get();
    }

    AB::Entities::Profession prof1_;
    AB::Entities::Profession prof2_;
};

}
