#pragma once

#include "Effect.h"
#include "Damage.h"

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;
class Item;

namespace Components {

class EffectsComp
{
private:
    static constexpr AB::Entities::EffectCategory SINGLEEFFECT_START = AB::Entities::EffectPreparation;
    static constexpr AB::Entities::EffectCategory SINGLEEFFECT_END = AB::Entities::EffectWeaponSpell;
    Actor& owner_;
    EffectList addedEffects_;
    EffectList removedEffects_;
    void RemoveAllOfCategory(AB::Entities::EffectCategory category);
private:
    // Events
    void OnGetCriticalHit(Actor* source, bool& value);
    void OnAttack(Actor* target, bool& value);
    void OnGettingAttacked(Actor* source, bool& value);
    void OnAttacked(Actor* source, DamageType type, int32_t damage, bool& success);
    void OnInterruptingAttack(bool& value);
    void OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& value);
    void OnUseSkill(Actor* target, Skill* skill, bool& value);
    void OnSkillTargeted(Actor* source, Skill* skill, bool& value);
    void OnKnockingDown(Actor* source, uint32_t time, bool& value);
    void OnHealing(Actor* source, int& value);
public:
    EffectsComp() = delete;
    explicit EffectsComp(Actor& owner);
    // non-copyable
    EffectsComp(const EffectsComp&) = delete;
    EffectsComp& operator=(const EffectsComp&) = delete;
    ~EffectsComp() = default;

    /// Add an effect.
    /// @param source The source of the effect, can be empty
    /// @param index The effect index
    /// @param time How long does the effect last. If 0 the script defines the duration.
    void AddEffect(std::shared_ptr<Actor> source, uint32_t index, uint32_t time);
    void DeleteEffect(uint32_t index);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);
    bool HasEffect(uint32_t index);
    bool HasEffectOf(AB::Entities::EffectCategory category);
    std::shared_ptr<Effect> GetLast(AB::Entities::EffectCategory category);
    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
    /// Get real cost of a skill
    /// \param skill The Skill
    /// \param activation Activation time
    /// \param energy Energy cost
    /// \param adrenaline Adrenaline cost
    /// \param overcast Causes overcast
    /// \param hp HP scarifies in percent of max health
    void GetSkillCost(Skill* skill,
        int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    void GetSkillRecharge(Skill* skill, int32_t& recharge);
    void GetDamage(DamageType type, int32_t& value, bool& critical);
    void GetAttackSpeed(Item* weapon, uint32_t& value);
    void GetAttackDamageType(DamageType& type);
    void GetAttackDamage(int32_t& value);
    void GetArmor(DamageType type, int& value);
    void GetArmorPenetration(float& value);
    void GetAttributeValue(uint32_t index, uint32_t& value);
    void GetResources(int& maxHealth, int& maxEnergy);

    EffectList effects_;
};

}
}
