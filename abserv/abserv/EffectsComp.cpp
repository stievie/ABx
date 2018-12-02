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
        addedEffects_.push_back(effect);
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
        removedEffects_.push_back((*it));
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

void EffectsComp::Write(Net::NetworkMessage& message)
{
    for (const auto& effect : removedEffects_)
    {
        message.AddByte(AB::GameProtocol::GameObjectEffectRemoved);
        message.Add<uint32_t>(owner_.id_);
        message.Add<uint32_t>(effect->data_.index);
    }
    removedEffects_.clear();

    for (const auto& effect : addedEffects_)
    {
        message.AddByte(AB::GameProtocol::GameObjectEffectAdded);
        message.Add<uint32_t>(owner_.id_);
        message.Add<uint32_t>(effect->data_.index);
        message.Add<uint32_t>(effect->GetTicks());
    }
    addedEffects_.clear();
}

}
}
