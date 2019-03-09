#pragma once

#include "Damage.h"

namespace Game {

class Actor;

namespace Components {

class AttackComp
{
private:
    Actor& owner_;
    bool attacking_;
    int64_t lastAttackTime_;
    uint32_t attackSpeed_;
    int32_t baseDamage_;
    DamageType damageType_;
    std::weak_ptr<Actor> target_;
public:
    explicit AttackComp(Actor& owner) :
        owner_(owner),
        attacking_(false),
        lastAttackTime_(0),
        attackSpeed_(0),
        baseDamage_(0),
        damageType_(DamageType::Unknown)
    { }
    ~AttackComp() = default;
    void Update(uint32_t timeElapsed);
    bool IsAttacking() const { return attacking_; }
    void Cancel();
    void Attack(std::shared_ptr<Actor> target);
    int64_t GetLastAttackTime() const { return lastAttackTime_; }
};

}
}
