#include "stdafx.h"
#include "EffectsComp.h"
#include "Actor.h"
#include "EffectManager.h"
#include "Subsystems.h"

namespace Game {
namespace Components {

void EffectsComp::AddEffect(std::shared_ptr<Actor> source, uint32_t index)
{
    RemoveEffect(index);

    auto effect = GetSubsystem<EffectManager>()->Get(index);
    if (effect)
    {
        // Effects are not stackable, are they?
        RemoveEffect(index);
        effects_.push_back(effect);
        effect->Start(source, owner_.GetThis<Actor>());
    }
}

void EffectsComp::DeleteEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::shared_ptr<Effect> const& current)
    {
        return current->data_.index == index;
    });
    if (it != effects_.end())
    {
        effects_.erase(it);
    }
}

void EffectsComp::RemoveEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::shared_ptr<Effect> const& current)
    {
        return current->data_.index == index;
    });
    if (it != effects_.end())
    {
        (*it)->Remove();
        effects_.erase(it);
    }
}

void EffectsComp::Update(uint32_t timeElapsed)
{
    for (const auto& effect : effects_)
    {
        if (effect->cancelled_ || effect->ended_)
        {
            DeleteEffect(effect->data_.index);
            continue;
        }
        effect->Update(timeElapsed);
    }
}

}
}
