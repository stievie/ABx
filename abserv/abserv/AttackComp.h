#pragma once

#include "Damage.h"
#include <AB/ProtocolCodes.h>

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
    AB::GameProtocol::AttackError lastError_;
    std::weak_ptr<Actor> target_;
public:
    AttackComp() = delete;
    explicit AttackComp(Actor& owner) :
        owner_(owner),
        attacking_(false),
        lastAttackTime_(0),
        attackSpeed_(0),
        baseDamage_(0),
        damageType_(DamageType::Unknown),
        lastError_(AB::GameProtocol::AttackErrorNone)
    { }
    // non-copyable
    AttackComp(const AttackComp&) = delete;
    AttackComp& operator=(const AttackComp&) = delete;
    ~AttackComp() = default;

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
    bool IsAttacking() const { return attacking_; }
    void Cancel();
    void Attack(std::shared_ptr<Actor> target);
    int64_t GetLastAttackTime() const { return lastAttackTime_; }
    bool IsAttackState() const;
    void SetAttackState(bool value);
};

}
}
