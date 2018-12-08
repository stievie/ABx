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
    std::weak_ptr<Actor> source_;
    std::weak_ptr<Actor> target_;
    // The real cos may be influenced by skills, armor, effects etc.
    int16_t realEnergy_;
    int16_t realAdrenaline_;
    int16_t realActivation_;
    int16_t realOvercast_;

    bool haveOnCancelled_;
    bool haveOnInterrupted_;
    AB::GameProtocol::SkillError lastError_;

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
        realEnergy_(0),
        realAdrenaline_(0),
        realActivation_(0),
        realOvercast_(0),
        haveOnCancelled_(false),
        haveOnInterrupted_(false),
        lastError_(AB::GameProtocol::SkillErrorNone),
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

    AB::GameProtocol::SkillError StartUse(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target);
    void CancelUse();
    /// Disable a skill for some time
    void Disable(uint32_t ticks)
    {
        if (recharged_ == 0)
            recharged_ = Utils::AbTick();
        recharged_ += ticks;
    }
    void Interrupt();
    AB::GameProtocol::SkillError GetLastError() const { return lastError_; }

    bool IsUsing() const { return (startUse_ != 0) && (Utils::AbTick() - startUse_ < activation_); }
    bool IsRecharged() const { return recharged_ <= Utils::AbTick(); }
    void SetRecharged(int64_t ticks) { recharged_ = ticks; }
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
    bool IsInRange(std::shared_ptr<Actor> target);
    Actor* GetSource()
    {
        if (auto s = source_.lock())
            return s.get();
        return nullptr;
    }
    Actor* GetTarget()
    {
        if (auto t = target_.lock())
            return t.get();
        return nullptr;
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
