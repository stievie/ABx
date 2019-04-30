#pragma once

#include "Utils.h"
#include <AB/Entities/Skill.h>
#include "Script.h"
#include "GameObject.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class Actor;

enum SkillEffect : uint32_t
{
    SkillEffectNone      = 0,
    SkillEffectResurrect = 1 << 1,
    SkillEffectHeal      = 1 << 2,
    SkillEffectProtect   = 1 << 3,
    SkillEffectDamage    = 1 << 4,
    SkillEffectSpeed     = 1 << 5,
    SkillEffectInterrupt = 1 << 6,
};

enum SkillTarget : uint32_t
{
    SkillTargetNone    = 0,
    SkillTargetSelf    = 1 << 1,
    SkillTargetTarget  = 1 << 2,
    SkillTargetAoe     = 1 << 3,
    SkillTargetParty   = 1 << 4,
};

class Skill
{
private:
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    int64_t startUse_;
    int64_t lastUse_;
    int64_t recharged_;
    Ranges range_;
    uint32_t skillEffect_;
    uint32_t effectTarget_;
    std::weak_ptr<Actor> source_;
    std::weak_ptr<Actor> target_;
    // The real cost may be influenced by skills, armor, effects etc.
    int32_t realEnergy_;
    int32_t realAdrenaline_;
    int32_t realActivation_;
    int32_t realOvercast_;
    int32_t realHp_;

    bool haveOnCancelled_;
    bool haveOnInterrupted_;
    AB::GameProtocol::SkillError lastError_;

    bool CanUseSkill(const std::shared_ptr<Actor>& source, const std::shared_ptr<Actor>& target);
    void InitializeLua();
    int _LuaGetType() const { return static_cast<int>(data_.type); }
    uint32_t _LuaGetIndex() const { return data_.index; }
    bool _LuaIsElite() const { return data_.isElite; }
    std::string _LuaGetName() const { return data_.name; }
public:
    static void RegisterLua(kaguya::State& state);

    Skill() = delete;
    explicit Skill(const AB::Entities::Skill& skill) :
        startUse_(0),
        lastUse_(0),
        recharged_(0),
        range_(Ranges::Aggro),
        skillEffect_(0),
        effectTarget_(0),
        realEnergy_(0),
        realAdrenaline_(0),
        realActivation_(0),
        realOvercast_(0),
        realHp_(0),
        haveOnCancelled_(false),
        haveOnInterrupted_(false),
        lastError_(AB::GameProtocol::SkillErrorNone),
        energy_(0),
        adrenaline_(0),
        activation_(0),
        recharge_(0),
        overcast_(0),
        hp_(0),
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

    AB::GameProtocol::SkillError StartUse(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target);
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

    bool IsUsing() const { return (startUse_ != 0) && (Utils::TimePassed(startUse_) < static_cast<uint32_t>(activation_)); }
    bool IsRecharged() const { return recharged_ <= Utils::Tick(); }
    void SetRecharged(int64_t ticks) { recharged_ = ticks; }
    bool IsType(AB::Entities::SkillType type) const
    {
        return (data_.type & type) == type;
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
    bool IsInRange(Actor* target);
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

    int32_t energy_;
    int32_t adrenaline_;
    int32_t activation_;
    int32_t recharge_;
    int32_t overcast_;
    /// HP sacrifice
    int32_t hp_;

    AB::Entities::Skill data_;
};

}
