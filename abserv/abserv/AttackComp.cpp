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

#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"
#include "Game.h"
#include "DamageComp.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

bool AttackComp::CheckRange()
{
    auto target = target_.lock();
    if (!target)
        return false;

    Item* item = owner_.GetWeapon();
    if (item)
    {
        float dist = item->GetWeaponRange();
        if (owner_.GetDistance(target.get()) <= dist)
            return true;
    }
    return owner_.IsInRange(Ranges::Touch, target.get());
}

void AttackComp::StartHit(Actor& target)
{
    // New attack
    attackSpeed_ = owner_.GetAttackSpeed();
    interrupted_ = false;
    owner_.FaceObject(&target);
    if (Utils::TimeElapsed(lastAttackTime_) >= attackSpeed_ / 2)
    {
        lastAttackTime_ = Utils::Tick();
        hitting_ = true;
        FireWeapon(target);
        damageType_ = owner_.GetAttackDamageType();
    }
}

void AttackComp::Hit(Actor& target)
{
    // Done attack -> apply damage
    hitting_ = false;
    if (interrupted_)
    {
        lastError_ = AB::GameProtocol::AttackErrorInterrupted;
        owner_.CallEvent<void(void)>(EVENT_ON_INTERRUPTEDATTACK);
    }
    else
    {
        const float criticalChance = owner_.GetCriticalChance(&target);
        auto rnd = GetSubsystem<Crypto::Random>();
        bool critical = criticalChance >= rnd->GetFloat();
        // Critical hit -> always weapons max damage
        int32_t damage = owner_.GetAttackDamage(critical);
        // Source effects may modify the damage
        owner_.effectsComp_->GetDamage(damageType_, damage, critical);
        bool canGettingAttacked = true;
        target.CallEvent<void(Actor*, DamageType, int32_t, bool&)>(EVENT_ON_ATTACKED,
            &owner_, damageType_, damage, canGettingAttacked);
        if (canGettingAttacked)
        {
            // Some effects may prevent attacks, e.g. blocking
            if (critical)
                // Some effect may prevent critical hits
                target.CallEvent<void(Actor*,bool&)>(EVENT_ON_GET_CRITICAL_HIT, &owner_, critical);
            if (critical)
                damage = static_cast<int>(static_cast<float>(damage) * std::sqrt(2.0f));
            target.damageComp_->ApplyDamage(&owner_, 0, damageType_, damage, owner_.GetArmorPenetration(), true);
        }
        else
        {
            lastError_ = AB::GameProtocol::AttackErrorInterrupted;
            owner_.CallEvent<void(void)>(EVENT_ON_INTERRUPTEDATTACK);
        }
    }
}

void AttackComp::FireWeapon(Actor& target)
{
    auto* weapon = owner_.GetWeapon();
    if (!weapon || !weapon->IsWeaponProjectile())
        return;
    owner_.GetGame()->AddProjectile(weapon->data_.spawnItemUuid,
        owner_.GetPtr<Actor>(), target.GetPtr<Actor>());
    if (!owner_.IsObjectInSight(target))
    {
        lastError_ = AB::GameProtocol::AttackErrorTargetObstructed;
    }
}

void AttackComp::MoveToTarget(std::shared_ptr<Actor> target)
{
    if (!owner_.autorunComp_->IsAutoRun())
    {
        Item* item = owner_.GetWeapon();
        const float dist = item ? item->GetWeaponRange() : RANGE_TOUCH;
        if (owner_.autorunComp_->Follow(target, false, dist))
        {
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureState::Moving);
            owner_.autorunComp_->SetAutoRun(true);
        }
        else
        {
            // No way to get to the target
            attacking_ = false;
            SetAttackState(false);
        }
    }
}

void AttackComp::Update(uint32_t /* timeElapsed */)
{
    if (!attacking_ || pause_)
        return;

    auto target = target_.lock();
    if (target)
    {
        if (target->IsDead())
        {
            // We can stop hitting to this target now :(
            // Poor target!
            attacking_ = false;
            SetAttackState(false);
            return;
        }
    }
    else
    {
        // Gone
        attacking_ = false;
        SetAttackState(false);
        return;
    }
    // We need to move to the target
    if (!CheckRange())
    {
        MoveToTarget(target);
        return;
    }
    else
    {
        owner_.autorunComp_->Reset();
        SetAttackState(true);
    }

    // We are in range of the target -> can start attacking it
    if (IsAttackState())
    {
        if (!hitting_)
        {
            StartHit(*target);
        }
        else
        {
            // Now we are really attacking. This can be interrupted.
            if (Utils::TimeElapsed(lastAttackTime_) >= attackSpeed_)
                Hit(*target);
        }
    }
}

void AttackComp::Write(Net::NetworkMessage& message)
{
    if (lastError_ != AB::GameProtocol::AttackErrorNone)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectAttackFailure);
        AB::Packets::Server::ObjectAttackFailure packet = {
            owner_.id_,
            static_cast<uint8_t>(lastError_)
        };
        AB::Packets::Add(packet, message);
        lastError_ = AB::GameProtocol::AttackErrorNone;
    }
}

void AttackComp::Cancel()
{
    attacking_ = false;
    SetAttackState(false);
}

bool AttackComp::Attack(std::shared_ptr<Actor> target, bool ping)
{
    bool canAttack = true;
    owner_.CallEvent<void(Actor*,bool&)>(EVENT_ON_ATTACK, target.get(), canAttack);
    if (!canAttack)
    {
        lastError_ = AB::GameProtocol::AttackErrorInvalidTarget;
        return false;
    }
    if (!target)
    {
        // Attack needs a target
        lastError_ = AB::GameProtocol::AttackErrorInvalidTarget;
        return false;
    }
    bool canGettingAttacked = true;
    target->CallEvent<void(Actor*, bool&)>(EVENT_ON_GETTING_ATTACKED, &owner_, canGettingAttacked);
    if (target->IsUndestroyable() && canGettingAttacked)
    {
        // Can not attack an destroyable target
        lastError_ = AB::GameProtocol::AttackErrorTargetUndestroyable;
        return false;
    }

    target_ = target;
    if (ping)
        owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
            target ? target->id_ : 0, AB::GameProtocol::ObjectCallTypeAttack, 0);
    attacking_ = true;
    lastAttackTime_ = 0;
    return true;
}

bool AttackComp::IsAttackingTarget(const Actor* target) const
{
    if (!IsAttackState())
        return false;
    if (!target)
        return false;
    if (auto t = target_.lock())
        return t->id_ == target->id_;
    return false;
}

bool AttackComp::IsAttackState() const
{
    return owner_.stateComp_.GetState() == AB::GameProtocol::CreatureState::Attacking;
}

void AttackComp::SetAttackState(bool value)
{
    if (IsAttackState() != value)
    {
        if (value)
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureState::Attacking);
        else
            owner_.stateComp_.Reset();
    }
}

bool AttackComp::Interrupt()
{
    if (hitting_)
    {
        interrupted_ = true;
        return true;
    }
    return false;
}

void AttackComp::Pause(bool value)
{
    pause_ = value;
}

bool AttackComp::IsTarget(const Actor* target) const
{
    if (!target)
        return false;
    if (const auto t = target_.lock())
        return t->id_ == target->id_;
    return false;
}

}
}
