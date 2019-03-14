#pragma once

#include "Effect.h"
#include "Damage.h"

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
    void RemoveAllOfCategory(AB::Entities::EffectCategory categroy);
public:
    EffectsComp() = delete;
    explicit EffectsComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    EffectsComp(const EffectsComp&) = delete;
    EffectsComp& operator=(const EffectsComp&) = delete;
    ~EffectsComp() = default;

    void AddEffect(std::shared_ptr<Actor> source, uint32_t index);
    void DeleteEffect(uint32_t index);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);
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
        int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp) const;
    void GetDamage(DamageType type, int32_t& value) const;
    void GetAttackSpeed(Item* weapon, uint32_t& value) const;
    void GetAttackDamageType(DamageType& type) const;
    void GetAttackDamage(int32_t& value) const;
    void CanAttack(bool& value) const;
    void CanBeAttacked(bool& value) const;
    void CanUseSkill(bool& value) const;
    void CanBeSkillTarget(bool& value) const;

    EffectList effects_;
};

}
}
