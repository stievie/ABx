#pragma once

#include <AB/Entities/Skill.h>
#include "Script.h"
#include "GameObject.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class Actor;
class SkillBar;

enum SkillEffect : uint32_t
{
    SkillEffectNone       = 0,
    SkillEffectResurrect  = 1 << 1,
    SkillEffectHeal       = 1 << 2,
    SkillEffectProtect    = 1 << 3,
    SkillEffectDamage     = 1 << 4,
    SkillEffectSpeed      = 1 << 5,
    SkillEffectInterrupt  = 1 << 6,
    SkillEffectGainEnergy = 1 << 7,
};

enum SkillTarget : uint32_t
{
    SkillTargetNone    = 0,
    SkillTargetSelf    = 1 << 1,
    SkillTargetTarget  = 1 << 2,              // Selected target
    SkillTargetAoe     = 1 << 3,
    SkillTargetParty   = 1 << 4,
};

enum class CostType
{
    Activation,
    Energy,
    Adrenaline,
    HpSacrify,
};

class Skill
{
    friend class SkillBar;
private:
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    int64_t startUse_{ 0 };
    int64_t lastUse_{ 0 };
    int64_t recharged_{ 0 };
    Ranges range_{ Ranges::Aggro };
    uint32_t skillEffect_{ SkillEffectNone };
    uint32_t effectTarget_{ SkillTargetNone };
    bool interrupts_{ false };
    AB::Entities::SkillType canInterrupt_{ AB::Entities::SkillTypeSkill };
    std::weak_ptr<Actor> source_;
    std::weak_ptr<Actor> target_;
    // The real cost may be influenced by skills, armor, effects etc.
    int32_t realEnergy_{ 0 };
    int32_t realAdrenaline_{ 0 };
    int32_t realActivation_{ 0 };
    int32_t realOvercast_{ 0 };
    int32_t realHp_{ 0 };

    bool haveOnCancelled_{ false };
    bool haveOnInterrupted_{ false };
    AB::GameProtocol::SkillError lastError_{ AB::GameProtocol::SkillErrorNone };

    bool CanUseSkill(Actor& source, Actor* target);
    void InitializeLua();
    int _LuaGetType() const { return static_cast<int>(data_.type); }
    uint32_t _LuaGetIndex() const { return data_.index; }
    bool _LuaIsElite() const { return data_.isElite; }
    std::string _LuaGetName() const { return data_.name; }
    int32_t GetActivation(Actor& source, int32_t activation);
    // Only SkillBar may use this
    AB::GameProtocol::SkillError StartUse(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target);
public:
    static void RegisterLua(kaguya::State& state);

    Skill() = delete;
    explicit Skill(const AB::Entities::Skill& skill) :
        data_(skill)
    {
        InitializeLua();
    }
    // non-copyable
    Skill(const Skill&) = delete;
    Skill& operator=(const Skill&) = delete;
    ~Skill() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);

    void CancelUse();
    /// Disable a skill for some time
    void Disable(uint32_t ticks)
    {
        if (recharged_ == 0)
            recharged_ = Utils::Tick();
        recharged_ += ticks;
    }
    bool Interrupt();
    AB::GameProtocol::SkillError GetLastError() const { return lastError_; }

    bool IsUsing() const { return (startUse_ != 0) && (Utils::TimeElapsed(startUse_) < static_cast<uint32_t>(activation_)); }
    bool IsRecharged() const { return recharged_ <= Utils::Tick(); }
    void SetRecharged(int64_t ticks) { recharged_ = ticks; }
    bool IsType(AB::Entities::SkillType type) const
    {
        return (data_.type & type) == type;
    }
    bool CanInterrupt(AB::Entities::SkillType type) const
    {
        return interrupts_ && ((canInterrupt_ & type) == type);
    }
    /// Does a skill change the creature state.
    bool IsChangingState() const
    {
        return !IsType(AB::Entities::SkillTypeStance) &&
            !IsType(AB::Entities::SkillTypeFlashEnchantment) &&
            !IsType(AB::Entities::SkillTypeShout);
    }
    uint32_t GetIndex() const { return data_.index; }
    bool HasEffect(SkillEffect effect) const { return (skillEffect_ & effect) == effect; }
    bool HasTarget(SkillTarget t) const { return (effectTarget_ & t) == t; }
    float CalculateCost(const std::function<float(CostType)>& importanceCallback) const;
    bool IsInRange(const Actor* target) const;
    Ranges GetRange() const { return range_; }
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
    int32_t GetRealEnergy() const { return realEnergy_; }
    int32_t GetRealAdrenaline() const { return realAdrenaline_; }
    int32_t GetRealActivation() const { return realActivation_; }
    int32_t GetRealOvercast() const { return realOvercast_; }
    int32_t GetRealHp() const { return realHp_; }

    int64_t GetLastUse() const { return	lastUse_; }

    void AddRecharge(int32_t ms);

    int32_t energy_{ 0 };
    int32_t adrenaline_{ 0 };
    int32_t activation_{ 0 };
    int32_t recharge_{ 0 };
    int32_t overcast_{ 0 };
    /// HP sacrifice
    int32_t hp_{ 0 };

    AB::Entities::Skill data_;
};

}
