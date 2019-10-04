#pragma once

#include <memory>
#include "Damage.h"
#include <AB/ProtocolCodes.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class AttackComp
{
private:
    Actor& owner_;
    /// Also includes running to the target
    bool attacking_{ false };
    bool hitting_{ false };
    bool pause_{ false };
    int64_t lastAttackTime_{ 0 };
    uint32_t attackSpeed_{ 0 };
    DamageType damageType_{ DamageType::Unknown };
    AB::GameProtocol::AttackError lastError_{ AB::GameProtocol::AttackErrorNone };
    bool interrupted_{ false };
    std::weak_ptr<Actor> target_;
    bool CheckRange();
    void StartHit(Actor& target);
    void Hit(Actor& target);
    void FireWeapon(Actor& target);
public:
    AttackComp() = delete;
    explicit AttackComp(Actor& owner) :
        owner_(owner)
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
    void SetAttackError(AB::GameProtocol::AttackError error) { lastError_ = error; }
};

}
}
