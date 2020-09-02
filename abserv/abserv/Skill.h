/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "GameObject.h"
#include <AB/Entities/Skill.h>
#include <AB/ProtocolCodes.h>
#include <eastl.hpp>
#include <sa/Bits.h>
#include <sa/Noncopyable.h>
#include <sa/time.h>

namespace Game {

class Actor;
class SkillBar;

enum SkillEffect : uint32_t
{
    SkillEffectNone              = 0,
    SkillEffectResurrect         = 1 <<  1,
    SkillEffectHeal              = 1 <<  2,
    SkillEffectProtect           = 1 <<  3,
    SkillEffectDamage            = 1 <<  4,
    SkillEffectSpeed             = 1 <<  5,
    SkillEffectInterrupt         = 1 <<  6,
    SkillEffectGainEnergy        = 1 <<  7,
    SkillEffectRemoveEnchantment = 1 <<  8,
    SkillEffectRemoveHex         = 1 <<  9,
    SkillEffectRemoveStance      = 1 << 10,
};

enum SkillEffectTarget : uint32_t
{
    SkillTargetNone    = 0,
    SkillTargetSelf    = 1 << 1,
    SkillTargetTarget  = 1 << 2,              // Selected target
    SkillTargetAoe     = 1 << 3,
    SkillTargetParty   = 1 << 4,
};

// Skills may or may not require a target
enum SkillTargetType : uint32_t
{
    SkillTargetTypeNone = 0,
    SkillTargetTypeSelf = 1,
    SkillTargetTypeAllyAndSelf = 2,
    SkillTargetTypeAllyWithoutSelf = 3,
    SkillTargetTypeFoe = 4,
};

enum class CostType
{
    Activation,
    Energy,
    Adrenaline,
    HpSacrify,
    Recharge,
};

class Skill
{
    NON_COPYABLE(Skill)
    friend class SkillBar;
private:
    kaguya::State luaState_;
    int64_t startUse_{ 0 };
    int64_t lastUse_{ 0 };
    int64_t recharged_{ 0 };
    Ranges range_{ Ranges::Aggro };
    uint32_t skillEffect_{ SkillEffectNone };
    uint32_t effectTarget_{ SkillTargetNone };
    SkillTargetType targetType_{ SkillTargetTypeNone };
    AB::Entities::SkillType canInterrupt_{ AB::Entities::SkillTypeSkill };
    ea::weak_ptr<Actor> source_;
    ea::weak_ptr<Actor> target_;
    // The real cost may be influenced by skills, armor, effects etc.
    int32_t realEnergy_{ 0 };
    int32_t realAdrenaline_{ 0 };
    int32_t realActivation_{ 0 };
    int32_t realOvercast_{ 0 };
    int32_t realHp_{ 0 };

    bool haveOnCancelled_{ false };
    bool haveOnInterrupted_{ false };
    AB::GameProtocol::SkillError lastError_{ AB::GameProtocol::SkillError::None };

    bool CanUseSkill(Actor& source, Actor* target);
    void InitializeLua();
    int _LuaGetType() const { return static_cast<int>(data_.type); }
    uint32_t _LuaGetIndex() const { return data_.index; }
    bool _LuaIsElite() const { return data_.isElite; }
    std::string _LuaGetName() const { return data_.name; }
    uint32_t GetRecharge(uint32_t recharge);
    // Only SkillBar may use this
    AB::GameProtocol::SkillError StartUse(ea::shared_ptr<Actor> source, ea::shared_ptr<Actor> target);
public:
    static void RegisterLua(kaguya::State& state);

    explicit Skill(const AB::Entities::Skill& skill) :
        data_(skill)
    {
        InitializeLua();
    }
    // non-copyable
    ~Skill() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);

    /// Calls the `canUse` or `onStartUse` function to see if the skill can be used
    /// and/or makes sense to use it
    AB::GameProtocol::SkillError CanUse(Actor* source, Actor* target);
    void CancelUse();
    /// Disable a skill for some time
    void Disable(uint32_t ticks)
    {
        if (recharged_ == 0)
            recharged_ = sa::time::tick();
        recharged_ += ticks;
    }
    bool Interrupt();
    AB::GameProtocol::SkillError GetLastError() const { return lastError_; }

    bool IsUsing() const { return (startUse_ != 0) && (sa::time::time_elapsed(startUse_) < static_cast<uint32_t>(activation_)); }
    bool IsRecharged() const { return recharged_ <= sa::time::tick(); }
    void SetRecharged(int64_t ticks) { recharged_ = ticks; }
    bool IsType(AB::Entities::SkillType type) const
    {
        // Unfortunately AB::Entities::SkillTypeSkill = 0 and 0 & 0 = 0 :/
        if (data_.type == AB::Entities::SkillTypeSkill || type == AB::Entities::SkillTypeSkill)
            return true;
        return sa::bits::is_set(data_.type, type);
    }
    bool CanUseOnAlly() const
    {
        return targetType_ == SkillTargetTypeNone || targetType_ == SkillTargetTypeAllyAndSelf || targetType_ == SkillTargetTypeAllyWithoutSelf;
    }
    bool CanUseOnFoe() const
    {
        return targetType_ == SkillTargetTypeNone || targetType_ == SkillTargetTypeFoe;
    }
    bool CanUseOnTarget(const Actor& source, const Actor* target) const;
    bool NeedsTarget() const
    {
        return targetType_ != SkillTargetTypeNone;
    }
    // Returns true if this skill can interrupt skills of type
    bool CanInterrupt(AB::Entities::SkillType type) const
    {
        if (!HasEffect(SkillEffectInterrupt))
            return false;
        if (canInterrupt_ == type)
            return true;
        return sa::bits::is_set(canInterrupt_, type);
    }
    /// Does a skill change the creature state.
    bool IsChangingState() const
    {
        return !IsType(AB::Entities::SkillTypeStance) &&
            !IsType(AB::Entities::SkillTypeFlashEnchantment) &&
            !IsType(AB::Entities::SkillTypeShout);
    }
    uint32_t GetIndex() const { return data_.index; }
    bool HasEffect(SkillEffect effect) const { return sa::bits::is_set(skillEffect_, effect); }
    bool HasTarget(SkillEffectTarget target) const { return sa::bits::is_set(effectTarget_, target); }
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

    const std::string& GetName() const { return data_.name; }

    int32_t energy_{ 0 };
    int32_t adrenaline_{ 0 };
    int32_t activation_{ 0 };
    uint32_t recharge_{ 0 };
    int32_t overcast_{ 0 };
    /// HP sacrifice
    int32_t hp_{ 0 };

    AB::Entities::Skill data_;
};

}
