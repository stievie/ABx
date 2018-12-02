#pragma once

#include "Effect.h"

namespace Game {

class Actor;

namespace Components {

class EffectsComp
{
private:
    Actor& owner_;
    EffectList addedEffects_;
    EffectList removedEffects_;
public:
    explicit EffectsComp(Actor& owner) :
        owner_(owner)
    { }
    ~EffectsComp() = default;

    void AddEffect(std::shared_ptr<Actor> source, uint32_t index);
    void DeleteEffect(uint32_t index);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);
    std::shared_ptr<Effect> GetLast(AB::Entities::EffectCategory category);
    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);

    EffectList effects_;
};

}
}
