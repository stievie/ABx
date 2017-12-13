#include "stdafx.h"
#include "Creature.h"
#include <algorithm>
#include "EffectManager.h"

#include "DebugNew.h"

namespace Game {

void Creature::RegisterLua(kaguya::State& state)
{
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        .addFunction("GetLevel", &Creature::GetLevel)

        .addFunction("GetSpeed", &Creature::GetSpeed)
        .addFunction("SetSpeed", &Creature::SetSpeed)
        .addFunction("GetEnergy", &Creature::GetEnergy)
        .addFunction("SetEnergy", &Creature::SetEnergy)
        /*        .addProperty("Energy", &Creature::GetEnergy, &Creature::SetEnergy)
        .addProperty("Health", &Creature::GetHealth, &Creature::SetHealth)
        .addProperty("Adrenaline", &Creature::GetAdrenaline, &Creature::SetAdrenaline)
        .addProperty("Overcast", &Creature::GetOvercast, &Creature::SetOvercast)
        .addProperty("Skills", &Creature::GetSkill)
        .addFunction("AddEffect", &Creature::AddEffectByName)*/
    );
}

Creature::Creature() :
    GameObject(),
    creatureState_(CreatureStateIdle)
{
}

void Creature::AddEffect(uint32_t id, uint32_t ticks)
{
    RemoveEffect(id);

    auto effect = EffectManager::Instance.Get(id);
    if (effect)
    {
        effects_.push_back(std::move(effect));
        effect->Start(GetThis<Creature>(), ticks);
    }
}

void Creature::AddEffectByName(const std::string& name, uint32_t ticks)
{
    uint32_t id = EffectManager::Instance.GetEffectId(name);
    AddEffect(id, ticks);
}

void Creature::DeleteEffect(uint32_t id)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::unique_ptr<Effect> const& current)
    {
        return current->id_ == id;
    });
    if (it != effects_.end())
    {
        effects_.erase(it);
    }
}

void Creature::RemoveEffect(uint32_t id)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::unique_ptr<Effect> const& current)
    {
        return current->id_ == id;
    });
    if (it != effects_.end())
    {
        (*it)->Remove();
        DeleteEffect((*it)->id_);
    }
}

void Creature::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);

    Skill* skill = nullptr;
    uint32_t targetId = 0;

    InputItem input;
    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputTypeMove:
            creatureState_ = CreatureStateMoving;
            break;
        case InputTypeAttack:
            creatureState_ = CreatureStateAttacking;
            break;
        case InputTypeUseSkill:
        {
            uint32_t skillIndex = static_cast<uint32_t>(input.data[InputDataSkillIndex].GetInt());
            skill = GetSkill(skillIndex);
            if (skill)
            {
                // Only stances do not change the state
                if (!skill->IsType(SkillTypeStance) && !skill->IsType(SkillTypeFlashEnchantment))
                    creatureState_ = CreatureStateUsingSkill;
            }
            break;
        }
        case InputTypeSelect:
            targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            break;
        case InputTypeNone:
            break;
        }
    }

    switch (creatureState_)
    {
    case CreatureStateIdle:
        break;
    case CreatureStateMoving:
        break;
    case CreatureStateUsingSkill:
        break;
    case CreatureStateAttacking:
        break;
    case CreatureStateEmote:
        break;
    }

    skills_.Update(timeElapsed);
    for (const auto& effect : effects_)
    {
        if (effect->cancelled_ || effect->ended_)
        {
            DeleteEffect(effect->id_);
            continue;
        }
        effect->Update(timeElapsed);
    }
}

}
