#pragma once

namespace Game {

class Actor;

namespace Components {

class AttackComp
{
private:
    Actor& owner_;
public:
    explicit AttackComp(Actor& owner) :
        owner_(owner)
    { }
    ~AttackComp() = default;
    void Update(uint32_t timeElapsed);
};

}
}
