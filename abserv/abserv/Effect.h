#pragma once

#include <memory>
#include <kaguya/kaguya.hpp>
#include "PropStream.h"
#include <AB/Entities/Effect.h>
#include <AB/Entities/Skill.h>
#include "Script.h"
#include "Damage.h"

namespace Game {

class Actor;
class Skill;
class Item;

enum EffectAttr : uint8_t
{
    EffectAttrId = 1,
    EffectAttrTicks,

    // For serialization
    EffectAttrEnd = 254
};

class Effect
{
private:
    enum Function : uint32_t
    {
        FunctionNone         = 0,
        FunctionUpdate       = 1,
        FunctionGetSkillCost = 1 << 1,
        FunctionGetDamage    = 1 << 2,
        FunctionGetAttackSpeed = 1 << 3,
        FunctionGetAttackDamageType = 1 << 4,
        FunctionGetAttackDamage = 1 << 5,
        FunctionOnAttack = 1 << 6,
        FunctionOnGettingAttacked = 1 << 7,
        FunctionOnUseSkill = 1 << 8,
        FunctionOnSkillTargeted = 1 << 9,
        FunctionOnAttacked = 1 << 10,
        FunctionOnInterruptingAttack = 1 << 11,
        FunctionOnInterruptingSkill = 1 << 12,
        FunctionOnKnockingDown = 1 << 13,
        FunctionOnHealing = 1 << 14,
        FunctionOnGetCriticalHit = 1 << 15,
        FunctionGetArmor = 1 << 16,
        FunctionGetArmorPenetration = 1 << 17,
        FunctionGetAttributeValue = 1 << 18,
        FunctionGetResources = 1 << 19,
    };
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    std::weak_ptr<Actor> target_;
    std::weak_ptr<Actor> source_;
    bool persistent_{ false };
    uint32_t functions_{ FunctionNone };
    /// Internal effects are not visible to the player, e.g. Effects from the equipments (+armor from Armor, Shield...).
    bool internal_{ false };
    bool UnserializeProp(EffectAttr attr, IO::PropReadStream& stream);
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
public:
    static void RegisterLua(kaguya::State& state);

    Effect() = delete;
    explicit Effect(const AB::Entities::Effect& effect) :
        data_(effect),
        startTime_(0),
        endTime_(0),
        ticks_(0),
        ended_(false),
        cancelled_(false)
    {
        InitializeLua();
    }
    // non-copyable
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;
    ~Effect() = default;

    /// Gets saved to the DB when player logs out, e.g. Dishonored.
    bool IsPersistent() const
    {
        return persistent_;
    }
    bool IsInternal() const { return internal_; }
    uint32_t GetIndex() const { return data_.index; }

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);
    bool Start(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target, uint32_t time);
    /// Remove Effect before it ends
    void Remove();
    /// Get real cost of a skill
    /// \param skill The Skill
    /// \param activation Activation time
    /// \param energy Energy cost
    /// \param adrenaline Adrenaline cost
    /// \param overcast Causes overcast
    /// \param hp HP scarifies in percent of max health
    void GetSkillCost(Skill* skill,
        int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    /// Get real damage. It may be in-/decreased by some effects on the *Target*. This is called when the damage is applied to the target.
    void GetDamage(DamageType type, int32_t& value, bool& critical);
    void GetAttackSpeed(Item* weapon, uint32_t& value);
    void GetAttackDamageType(DamageType& type);
    void GetArmor(DamageType type, int& value);
    void GetArmorPenetration(float& value);
    void GetAttributeValue(uint32_t index, uint32_t& value);
    /// Attack damage may be in-/decreased by effects on the *Source*. This is called when the source starts attacking.
    void GetAttackDamage(int32_t& value);
    void GetRecources(int& maxHealth, int& maxEnergy);
    /// Some effect may make the attacker unable to attack. The target is being attacked.
    void OnAttack(Actor* source, Actor* target, bool& value);
    void OnAttacked(Actor* source, Actor* target, DamageType type, int32_t damage, bool& success);
    /// Checks whether the owner can be attacked
    void OnGettingAttacked(Actor* source, Actor* target, bool& value);
    void OnUseSkill(Actor* source, Actor* target, Skill* skill, bool& value);
    void OnSkillTargeted(Actor* source, Actor* target, Skill* skill, bool& value);
    void OnInterruptingAttack(bool& value);
    void OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& value);
    void OnKnockingDown(Actor* source, Actor* target, uint32_t time, bool& value);
    void OnGetCriticalHit(Actor* source, Actor* target, bool& value);
    /// Targets gets healed by source
    void OnHealing(Actor* source, Actor* target, int& value);

    bool Serialize(IO::PropWriteStream& stream);
    bool Unserialize(IO::PropReadStream& stream);

    AB::Entities::Effect data_;

    int64_t startTime_;
    int64_t endTime_;
    /// Duration
    uint32_t ticks_;
    bool ended_;
    bool cancelled_;

    int64_t GetStartTime() const { return startTime_; }
    int64_t GetEndTime() const { return endTime_; }
    uint32_t GetTicks() const { return ticks_; }
};

/// effects are fist-in-last-out
typedef std::vector<std::shared_ptr<Effect>> EffectList;

enum ConditionType : uint8_t
{
    ConditionNone = 0,
    ConditionDazed,
    ConditionCrackedArmor,
    ConditionBlind,
    ConditionBleeding,
    ConditionBurning,
    ConditionPoison,
    ConditionDesease,
    ConditionWeakness,
    ConditionDeepWound,
    ConditionCrippled
};

}

