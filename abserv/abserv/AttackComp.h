/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <memory>
#include <abshared/Damage.h>
#include <AB/ProtocolCodes.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class AttackComp
{
    NON_COPYABLE(AttackComp)
    NON_MOVEABLE(AttackComp)
private:
    Actor& owner_;
    /// Also includes running to the target
    bool attacking_{ false };
    bool hitting_{ false };
    bool pause_{ false };
    int64_t lastAttackTime_{ 0 };
    uint32_t attackSpeed_{ 0 };
    uint32_t lastAttackSpeed_{ 0 };
    bool attackSpeedDirty_{ false };
    DamageType damageType_{ DamageType::Unknown };
    AB::GameProtocol::AttackError lastError_{ AB::GameProtocol::AttackError::None };
    bool interrupted_{ false };
    std::weak_ptr<Actor> target_;
    bool CheckRange();
    void StartHit(Actor& target);
    void Hit(Actor& target);
    void FireWeapon(Actor& target);
    void MoveToTarget(std::shared_ptr<Actor> target);
public:
    AttackComp() = delete;
    explicit AttackComp(Actor& owner) :
        owner_(owner)
    { }
    ~AttackComp() = default;

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
    bool IsHitting() const { return hitting_; }
    void Cancel();
    bool Attack(std::shared_ptr<Actor> target, bool ping);
    int64_t GetLastAttackTime() const { return lastAttackTime_; }
    bool IsAttackState() const;
    void SetAttackState(bool value);
    bool IsAttackingTarget(const Actor* target) const;
    bool Interrupt();
    void Pause(bool value = true);
    bool IsTarget(const Actor* target) const;
    Actor* GetCurrentTarget() const
    {
        if (auto t = target_.lock())
            return t.get();
        return nullptr;
    }
    void SetAttackError(AB::GameProtocol::AttackError error) { lastError_ = error; }
};

}
}
