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
    /// Also includes running to the target
    bool attacking_;
    bool hitting_;
    bool pause_;
    int64_t lastAttackTime_;
    uint32_t attackSpeed_;
    DamageType damageType_;
    AB::GameProtocol::AttackError lastError_;
    bool interrupted_;
    std::weak_ptr<Actor> target_;
    bool CheckRange();
public:
    AttackComp() = delete;
    explicit AttackComp(Actor& owner) :
        owner_(owner),
        attacking_(false),
        hitting_(false),
        pause_(false),
        lastAttackTime_(0),
        attackSpeed_(0),
        damageType_(DamageType::Unknown),
        lastError_(AB::GameProtocol::AttackErrorNone),
        interrupted_(false)
    { }
    // non-copyable
    AttackComp(const AttackComp&) = delete;
    AttackComp& operator=(const AttackComp&) = delete;
    ~AttackComp() = default;

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
    bool IsHitting() const { return hitting_; }
    void Cancel();
    void Attack(std::shared_ptr<Actor> target, bool ping);
    int64_t GetLastAttackTime() const { return lastAttackTime_; }
    bool IsAttackState() const;
    void SetAttackState(bool value);
    bool IsAttackingTarget(Actor* target) const;
    bool Interrupt();
    void Pause(bool value = true);
    bool IsTarget(const Actor* target) const;
};

}
}
