#pragma once

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
    void Update(uint32_t timeElapsed);
};

}
}
