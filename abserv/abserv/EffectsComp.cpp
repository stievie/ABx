#include "stdafx.h"
#include "EffectsComp.h"
#include "Actor.h"
#include "EffectManager.h"
#include "Subsystems.h"
#include "Item.h"

namespace Game {
namespace Components {

void EffectsComp::RemoveAllOfCategory(AB::Entities::EffectCategory categroy)
{
    const auto check = [categroy](const std::shared_ptr<Effect>& current)
    {
        return current->data_.category == categroy;
    };

    auto it = std::find_if(effects_.begin(), effects_.end(), check);
    while (it != effects_.end())
    {
        (*it)->Remove();
        removedEffects_.push_back((*it));
        effects_.erase(it);
        it = std::find_if(effects_.begin(), effects_.end(), check);
    }
}

void EffectsComp::AddEffect(std::shared_ptr<Actor> source, uint32_t index)
{
    auto effect = GetSubsystem<EffectManager>()->Get(index);
    if (effect)
    {
        // Effects are not stackable:
        /*
         * Eine Fertigkeit kann nicht mit sich selbst gestapelt werden, beispielsweise
         * kann man einen Gegner also nicht zweimal mit der gleichen Verhexung belegen,
         * sich selbst mehrfach durch ein und dieselbe Verzauberung schützen oder
         * den Bonus eines Schreis mehrfach erhalten.
         * https://www.guildwiki.de/wiki/Effektstapelung
         */
        RemoveEffect(index);
        // E.g. only one stance allowed
        if (effect->data_.category >= SINGLEEFFECT_START && effect->data_.category <= SINGLEEFFECT_END)
            RemoveAllOfCategory(effect->data_.category);
        if (effect->Start(source, owner_.GetThis<Actor>()))
        {
            effects_.push_back(effect);
            addedEffects_.push_back(effect);
        }
    }
}

void EffectsComp::DeleteEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [index](const std::shared_ptr<Effect>& current)
    {
        return current->data_.index == index;
    });
    if (it != effects_.end())
    {
        removedEffects_.push_back((*it));
        effects_.erase(it);
    }
}

void EffectsComp::RemoveEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [index](const std::shared_ptr<Effect>& current)
    {
        return current->data_.index == index;
    });
    if (it != effects_.end())
    {
        (*it)->Remove();
        removedEffects_.push_back((*it));
        effects_.erase(it);
    }
}

std::shared_ptr<Effect> EffectsComp::GetLast(AB::Entities::EffectCategory category)
{
    for (auto i = effects_.rbegin(); i != effects_.rend(); ++i)
    {
        if ((*i)->data_.category == category)
            return (*i);
    }
    return std::shared_ptr<Effect>();
}

void EffectsComp::Update(uint32_t timeElapsed)
{
    auto it = effects_.begin();
    while (it != effects_.end())
    {
        if ((*it)->cancelled_ || (*it)->ended_)
        {
            removedEffects_.push_back((*it));
            it = effects_.erase(it);
        }
        else
        {
            (*it)->Update(timeElapsed);
            ++it;
        }
    }
}

void EffectsComp::Write(Net::NetworkMessage& message)
{
    if (removedEffects_.size() != 0)
    {
        for (const auto& effect : removedEffects_)
        {
            if (effect->IsInternal())
                continue;
            message.AddByte(AB::GameProtocol::GameObjectEffectRemoved);
            message.Add<uint32_t>(owner_.id_);
            message.Add<uint32_t>(effect->data_.index);
        }
        removedEffects_.clear();
    }

    if (addedEffects_.size() != 0)
    {
        for (const auto& effect : addedEffects_)
        {
            if (effect->IsInternal())
                continue;
            message.AddByte(AB::GameProtocol::GameObjectEffectAdded);
            message.Add<uint32_t>(owner_.id_);
            message.Add<uint32_t>(effect->data_.index);
            message.Add<uint32_t>(effect->GetTicks());
        }
        addedEffects_.clear();
    }
}

void EffectsComp::GetSkillCost(Skill* skill,
    int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
    // Since equipments, attributes etc. add (hidden) effects to the actor, we need only ask the effects component, I think...
    for (const auto& effect : effects_)
    {
        effect->GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);
    }
}

void EffectsComp::GetDamage(DamageType type, int32_t& value, bool& critical)
{
    for (const auto& effect : effects_)
    {
        effect->GetDamage(type, value, critical);
    }
}

void EffectsComp::GetAttackSpeed(Item* weapon, uint32_t& value)
{
    for (const auto& effect : effects_)
    {
        effect->GetAttackSpeed(weapon, value);
    }
}

void EffectsComp::GetAttackDamageType(DamageType& type)
{
    for (const auto& effect : effects_)
    {
        effect->GetAttackDamageType(type);
    }
}

void EffectsComp::GetAttackDamage(int32_t& value)
{
    for (const auto& effect : effects_)
    {
        effect->GetAttackDamage(value);
    }
}

void EffectsComp::GetArmor(DamageType type, int& value)
{
    for (const auto& effect : effects_)
    {
        effect->GetArmor(type, value);
    }
}

void EffectsComp::GetArmorPenetration(float& value)
{
    for (const auto& effect : effects_)
    {
        effect->GetArmorPenetration(value);
    }
}

void EffectsComp::GetAttributeValue(uint32_t index, uint32_t& value)
{
    for (const auto& effect : effects_)
    {
        effect->GetAttributeValue(index, value);
    }
}

void EffectsComp::GetResources(int& maxHealth, int& maxEnergy)
{
    for (const auto& effect : effects_)
    {
        effect->GetRecources(maxHealth, maxEnergy);
    }
}

void EffectsComp::OnAttack(Actor* target, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnAttack(&owner_, target, value);
    }
}

void EffectsComp::OnAttacked(Actor* source, DamageType type, int32_t damage, bool& success)
{
    for (const auto& effect : effects_)
    {
        if (!success)
            return;
        effect->OnAttacked(source, &owner_, type, damage, success);
    }
}

void EffectsComp::OnGettingAttacked(Actor* source, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnGettingAttacked(source, &owner_, value);
    }
}

void EffectsComp::OnUseSkill(Actor* target, Skill* skill, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnUseSkill(&owner_, target, skill, value);
    }
}

void EffectsComp::OnSkillTargeted(Actor* source, Skill* skill, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnSkillTargeted(source, &owner_, skill, value);
    }
}

void EffectsComp::OnInterruptingAttack(bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnInterruptingAttack(value);
    }
}

void EffectsComp::OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnInterruptingSkill(type, skill, value);
    }
}

void EffectsComp::OnKnockingDown(Actor* source, uint32_t time, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnKnockingDown(source, &owner_, time, value);
    }
}

void EffectsComp::OnHealing(Actor* source, int& value)
{
    for (const auto& effect : effects_)
    {
        effect->OnHealing(source, &owner_, value);
    }
}

void EffectsComp::OnGetCriticalHit(Actor* source, bool& value)
{
    for (const auto& effect : effects_)
    {
        if (!value)
            return;
        effect->OnGetCriticalHit(source, &owner_, value);
    }
}

}
}
