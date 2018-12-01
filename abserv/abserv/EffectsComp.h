#pragma once

#include "Effect.h"

namespace Game {

class Actor;

namespace Components {

class EffectsComp
{
private:
    Actor& owner_;
public:
    explicit EffectsComp(Actor& owner) :
        owner_(owner)
    { }
    ~EffectsComp() = default;

    void AddEffect(std::shared_ptr<Actor> source, uint32_t index);
    void DeleteEffect(uint32_t index);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);
    void Update(uint32_t timeElapsed);

    EffectList effects_;
};

}
}
