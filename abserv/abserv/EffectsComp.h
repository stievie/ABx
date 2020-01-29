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

#include "Effect.h"
#include "Damage.h"
#include <sa/Noncopyable.h>
#include "Attributes.h"

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
    NON_COPYABLE(EffectsComp)
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
    void OnMorale(int morale);
public:
    EffectsComp() = delete;
    explicit EffectsComp(Actor& owner);
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
    void GetSkillRecharge(Skill* skill, uint32_t& recharge);
    void GetDamage(DamageType type, int32_t& value, bool& critical);
    void GetAttackSpeed(Item* weapon, uint32_t& value);
    void GetAttackDamageType(DamageType& type);
    void GetAttackDamage(int32_t& value);
    void GetArmor(DamageType type, int& value);
    void GetArmorPenetration(float& value);
    void GetAttributeValue(Attribute index, uint32_t& value);
    void GetResources(int& maxHealth, int& maxEnergy);

    EffectList effects_;
};

}
}
