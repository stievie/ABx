#pragma once

namespace Game {

class Actor;

namespace Components {

class AttackComp
{
private:
    Actor& owner_;
    bool attacking_;
public:
    explicit AttackComp(Actor& owner) :
        owner_(owner),
        attacking_(false)
    { }
    ~AttackComp() = default;
    void Update(uint32_t timeElapsed);
    bool IsAttacking() const { return attacking_; }
    void Cancel();
};

}
}
