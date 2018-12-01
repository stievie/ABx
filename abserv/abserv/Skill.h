#pragma once

#include "Utils.h"
#include <AB/Entities/Skill.h>
#include "Script.h"
#include "GameObject.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class Actor;

enum SkillEffect : uint16_t
{
    SkillEffectNone = 0,
    SkillEffectResurrect = 1 << 1,
    SkillEffectHeal = 1 << 2,
    SkillEffectProtect = 1 << 3,
    SkillEffectDamage = 1 << 4,
    SkillEffectSpeed = 1 << 5,
};

enum SkillTarget : uint16_t
{
    SkillTargetNone = 0,
    SkillTargetSelf = 1 << 1,
    SkillTargetTarget = 1 << 2,
    SkillTargetAoe = 1 << 3,
};

class Skill
{
private:
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    int64_t startUse_;
    int64_t recharged_;
    Ranges range_;
    SkillEffect skillEffect_;
    SkillTarget effectTarget;
    void InitializeLua();
    Actor* source_;
    Actor* target_;
    // The real cos may be influenced by skills, armor, effects etc.
    int16_t realEnergy_;
    int16_t realAdrenaline_;
    int16_t realActivation_;
    int16_t realOvercast_;

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
        skillEffect_(SkillEffectNone),
        effectTarget(SkillTargetNone),
        source_(nullptr),
        target_(nullptr),
        realEnergy_(0),
        realAdrenaline_(0),
        realActivation_(0),
        realOvercast_(0),
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

    AB::GameProtocol::SkillError StartUse(Actor* source, Actor* target);
    void CancelUse();
    /// Disable a skill for some time
    void Disable(uint32_t ticks)
    {
        if (recharged_ == 0)
            recharged_ = Utils::AbTick();
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
    bool HasTarget(SkillTarget t) const { return (effectTarget & t) == t; }
    bool IsInRange(Actor* target);
    Actor* GetSource()
    {
        return source_;
    }
    Actor* GetTarget()
    {
        return target_;
    }
    int16_t GetRealEnergy() const { return realEnergy_; }
    int16_t GetRealAdrenaline() const { return realAdrenaline_; }
    int16_t GetRealActivation() const { return realActivation_; }
    int16_t GetRealOvercast() const { return realOvercast_; }

    void AddRecharge(int16_t ms);

    AB::Entities::Skill data_;

    int16_t energy_;
    int16_t adrenaline_;
    int16_t activation_;
    int16_t recharge_;
    int16_t overcast_;
};

}
