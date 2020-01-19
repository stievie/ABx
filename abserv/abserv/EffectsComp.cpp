#include "stdafx.h"
#include "EffectsComp.h"
#include "Actor.h"
#include "EffectManager.h"
#include "Subsystems.h"
#include "Item.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

EffectsComp::EffectsComp(Actor& owner) :
    owner_(owner)
{
    owner_.SubscribeEvent<void(Actor*, bool&)>(EVENT_ON_GET_CRITICAL_HIT, std::bind(&EffectsComp::OnGetCriticalHit, this, std::placeholders::_1, std::placeholders::_2));
    owner_.SubscribeEvent<void(Actor*, DamageType, int32_t, bool&)>(EVENT_ON_ATTACKED, std::bind(
        &EffectsComp::OnAttacked, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    owner_.SubscribeEvent<void(Actor*, bool&)>(EVENT_ON_GETTING_ATTACKED, std::bind(&EffectsComp::OnGettingAttacked, this, std::placeholders::_1, std::placeholders::_2));
    owner_.SubscribeEvent<void(Actor*, bool&)>(EVENT_ON_ATTACK, std::bind(&EffectsComp::OnAttack, this, std::placeholders::_1, std::placeholders::_2));
    owner_.SubscribeEvent<void(bool&)>(EVENT_ON_INTERRUPTING_ATTACK, std::bind(&EffectsComp::OnInterruptingAttack, this, std::placeholders::_1));
    owner_.SubscribeEvent<void(AB::Entities::SkillType, Skill*, bool&)>(EVENT_ON_INTERRUPTING_SKILL, std::bind(&EffectsComp::OnInterruptingSkill, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    owner_.SubscribeEvent<void(Actor*, Skill*, bool&)>(EVENT_ON_USESKILL, std::bind(&EffectsComp::OnUseSkill, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    owner_.SubscribeEvent<void(Actor*, Skill*, bool&)>(EVENT_ON_SKILLTARGETED, std::bind(&EffectsComp::OnSkillTargeted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    owner_.SubscribeEvent<void(Actor*, uint32_t, bool&)>(EVENT_ON_KNOCKING_DOWN, std::bind(&EffectsComp::OnKnockingDown, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    owner_.SubscribeEvent<void(Actor*, int&)>(EVENT_ON_HEALING, std::bind(&EffectsComp::OnHealing, this, std::placeholders::_1, std::placeholders::_2));
    owner_.SubscribeEvent<void(int)>(EVENT_ON_INCMORALE, std::bind(&EffectsComp::OnMorale, this, std::placeholders::_1));
    owner_.SubscribeEvent<void(int)>(EVENT_ON_DECMORALE, std::bind(&EffectsComp::OnMorale, this, std::placeholders::_1));
}

void EffectsComp::OnMorale(int morale)
{
    if (morale != 0)
        AddEffect(std::shared_ptr<Actor>(), AB::Entities::EFFECT_INDEX_MORALE, 0);
    else
        RemoveEffect(AB::Entities::EFFECT_INDEX_MORALE);
}

void EffectsComp::RemoveAllOfCategory(AB::Entities::EffectCategory category)
{
    const auto check = [category](const std::shared_ptr<Effect>& current)
    {
        return current->data_.category == category;
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

bool EffectsComp::HasEffectOf(AB::Entities::EffectCategory category)
{
    const auto check = [category](const std::shared_ptr<Effect>& current)
    {
        return current->data_.category == category;
    };

    auto it = std::find_if(effects_.begin(), effects_.end(), check);
    return it != effects_.end();
}


void EffectsComp::AddEffect(std::shared_ptr<Actor> source, uint32_t index, uint32_t time)
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
        if (effect->Start(source, owner_.GetPtr<Actor>(), time))
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

bool EffectsComp::HasEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [index](const std::shared_ptr<Effect>& current)
    {
        return current->data_.index == index;
    });
    return it != effects_.end();
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
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectEffectRemoved);
            AB::Packets::Server::ObjectEffectRemoved packet = {
                owner_.id_,
                effect->data_.index
            };
            AB::Packets::Add(packet, message);
        }
        removedEffects_.clear();
    }

    if (addedEffects_.size() != 0)
    {
        for (const auto& effect : addedEffects_)
        {
            if (effect->IsInternal())
                continue;
            message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectEffectAdded);
            AB::Packets::Server::ObjectEffectAdded packet = {
                owner_.id_,
                effect->data_.index,
                effect->GetTicks()
            };
            AB::Packets::Add(packet, message);
        }
        addedEffects_.clear();
    }
}

void EffectsComp::GetSkillRecharge(Skill* skill, uint32_t& recharge)
{
    for (const auto& effect : effects_)
    {
        effect->GetSkillRecharge(skill, recharge);
    }
}

void EffectsComp::GetSkillCost(Skill* skill,
    int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
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
