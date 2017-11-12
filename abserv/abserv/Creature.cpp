#include "stdafx.h"
#include "Creature.h"
#include <algorithm>
#include "EffectManager.h"

namespace Game {

void Creature::RegisterLua(kaguya::State& state)
{
    GameObject::RegisterLua(state);
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        .addProperty("Speed", &Creature::GetSpeed, &Creature::SetSpeed)
        .addProperty("Energy", &Creature::GetEnergy, &Creature::SetEnergy)
        .addProperty("Health", &Creature::GetHealth, &Creature::SetHealth)
        .addProperty("Adrenaline", &Creature::GetAdrenaline, &Creature::SetAdrenaline)
        .addProperty("Overcast", &Creature::GetOvercast, &Creature::SetOvercast)
        .addProperty("Skills", &Creature::GetSkill)
        .addFunction("AddEffect", &Creature::AddEffect)
    );
}

void Creature::AddEffect(uint32_t id, uint32_t ticks)
{
    RemoveEffect(id);

    auto effect = EffectManager::Instance.Get(id);
    if (effect)
    {
        effects_.push_back(std::move(effect));
        effect->Start(this, ticks);
    }
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

void Creature::Update(uint32_t timeElapsed)
{
    GameObject::Update(timeElapsed);
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
