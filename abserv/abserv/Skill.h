#pragma once

#include "Utils.h"
#include <AB/Entities/Skill.h>
#include "Script.h"
#include "GameObject.h"

namespace Game {

class Actor;

enum SkillEffect : uint16_t
{
    SkillEffectNone = 0,
    SkillEffectResurrect = 1,
    SkillEffectHeal = 2,
    SkillEffectProtect = 3,
    SkillEffectDamage = 4,
};

enum SkillTarget : uint16_t
{
    SkillTargetNone = 0,
    SkillTargetSelf = 1 << 8,
    SkillTargetTarget = 2 << 8,
    SkillTargetAoe = 3 << 8,
};

class Skill
{
private:
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    int64_t startUse_;
    int64_t recharged_;
    Ranges range_;
    uint32_t skillEffect_;
    void InitializeLua();
    Actor* source_;
    Actor* target_;
    int _LuaGetType() const { return static_cast<int>(data_.type); }
    uint32_t _LuaGetIndex() const { return data_.index; }
    bool _LuaIsElite() const { return data_.isElite; }
    std::string _LuaGetName() const { return data_.name; }
public:
    static void RegisterLua(kaguya::State& state);

    Skill() = delete;
    explicit Skill(const AB::Entities::Skill& skill) :
        startUse_(0),
        recharged_(0),
        range_(Ranges::Aggro),
        skillEffect_(0),
        source_(nullptr),
        target_(nullptr),
        energy_(0),
        adrenaline_(0),
        activation_(0),
        recharge_(0),
        overcast_(0),
        data_(skill)
    {
        InitializeLua();
    }
    ~Skill() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);

    bool StartUse(Actor* source, Actor* target);
    void CancelUse();
    /// Disable a skill for some time
    void Disable(uint32_t ticks)
    {
        recharged_ += ticks;
    }
    void Interrupt();

    bool IsUsing() const { return (startUse_ != 0) && (startUse_ - Utils::AbTick() >= activation_); }
    bool IsRecharged() const { return recharged_ <= Utils::AbTick(); }
    bool IsType(AB::Entities::SkillType type)
    {
        return (data_.type & type) == type;
    }
    /// Does a skill change the creature state.
    bool IsChangingState()
    {
        return !IsType(AB::Entities::SkillTypeStance) &&
            !IsType(AB::Entities::SkillTypeFlashEnchantment) &&
            !IsType(AB::Entities::SkillTypeShout);
    }
    bool HasEffect(SkillEffect effect) const { return (skillEffect_ & effect) == effect; }
    bool HasTarget(SkillTarget t) const { return (skillEffect_ & t) == t; }
    bool IsInRange(Actor* target);
    Actor* GetSource()
    {
        return source_;
    }
    Actor* GetTarget()
    {
        return target_;
    }
    void AddRecharge(int16_t ms);

    AB::Entities::Skill data_;

    int16_t energy_;
    int16_t adrenaline_;
    int16_t activation_;
    int16_t recharge_;
    int16_t overcast_;
};

}
