#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"
#include "Subsystems.h"
#include "Random.h"
#include "Game.h"

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

void AttackComp::StartHit(Actor* target)
{
    // New attack
    attackSpeed_ = owner_.GetAttackSpeed();
    interrupted_ = false;
    owner_.FaceObject(target);
    if (Utils::TimeElapsed(lastAttackTime_) >= attackSpeed_ / 2)
    {
        lastAttackTime_ = Utils::Tick();
        hitting_ = true;
        FireWeapon(target);
        damageType_ = owner_.GetAttackDamageType();
    }
}

void AttackComp::Hit(Actor* target)
{
    // Done attack -> apply damage
    hitting_ = false;
    if (interrupted_)
    {
        lastError_ = AB::GameProtocol::AttackErrorInterrupted;
        owner_.OnInterruptedAttack();
    }
    else
    {
        const float criticalChance = owner_.GetCriticalChance(target);
        auto rnd = GetSubsystem<Crypto::Random>();
        bool critical = criticalChance >= rnd->GetFloat();
        // Critical hit -> always weapons max damage
        int32_t damage = owner_.GetAttackDamage(critical);
        // Source effects may modify the damage
        owner_.effectsComp_->GetDamage(damageType_, damage, critical);
        if (target)
        {
            if (target->OnAttacked(&owner_, damageType_, damage))
            {
                // Some effects may prevent attacks, e.g. blocking
                if (critical)
                    // Some effect may prevent critical hits
                    critical = target->OnGetCriticalHit(&owner_);
                if (critical)
                    damage = static_cast<int>(static_cast<float>(damage) * std::sqrt(2.0f));
                target->ApplyDamage(&owner_, 0, damageType_, damage, owner_.GetArmorPenetration());
            }
            else
            {
                lastError_ = AB::GameProtocol::AttackErrorInterrupted;
                owner_.OnInterruptedAttack();
            }
        }
        else
        {
            lastError_ = AB::GameProtocol::AttackErrorNoTarget;
            owner_.OnInterruptedAttack();
        }
    }
}

void AttackComp::FireWeapon(Actor* target)
{
    if (!target)
        return;

    auto* weapon = owner_.GetWeapon();
    if (!weapon || !weapon->IsWeaponProjectile())
        return;
    owner_.GetGame()->AddProjectile(weapon->data_.spawnItemUuid,
        owner_.GetThis<Actor>(), target->GetThis<Actor>());
    if (!owner_.IsObjectInSight(target))
    {
        lastError_ = AB::GameProtocol::AttackErrorTargetObstructed;
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
        if (!owner_.autorunComp_->IsAutoRun())
        {
            if (owner_.autorunComp_->Follow(target, false))
            {
                owner_.followedObject_ = target;
                owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                owner_.autorunComp_->SetAutoRun(true);
            }
            else
            {
                // No way to get to target
                attacking_ = false;
                SetAttackState(false);
            }
        }
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
            StartHit(target.get());
        }
        else
        {
            // Now we are really attacking. This can be interrupted.
            if (Utils::TimeElapsed(lastAttackTime_) >= attackSpeed_)
                Hit(target.get());
        }
    }
}

void AttackComp::Write(Net::NetworkMessage& message)
{
    if (lastError_ != AB::GameProtocol::AttackErrorNone)
    {
        message.AddByte(AB::GameProtocol::GameObjectAttackFailure);
        message.Add<uint32_t>(owner_.id_);
        message.AddByte(lastError_);
        lastError_ = AB::GameProtocol::AttackErrorNone;
    }
}

void AttackComp::Cancel()
{
    attacking_ = false;
    SetAttackState(false);
}

void AttackComp::Attack(std::shared_ptr<Actor> target, bool ping)
{
    if (!owner_.OnAttack(target.get()))
    {
        lastError_ = AB::GameProtocol::AttackErrorInvalidTarget;
        return;
    }
    if (!target)
    {
        // Attack needs a target
        lastError_ = AB::GameProtocol::AttackErrorInvalidTarget;
        return;
    }
    if (target->IsUndestroyable() && target->OnGettingAttacked(&owner_))
    {
        // Can not attack an destroyable target
        lastError_ = AB::GameProtocol::AttackErrorTargetUndestroyable;
        return;
    }

    target_ = target;
    if (ping)
        owner_.OnPingObject(target ? target->id_ : 0, AB::GameProtocol::ObjectCallTypeAttack, 0);
    attacking_ = true;
    lastAttackTime_ = 0;
}

bool AttackComp::IsAttackingTarget(Actor* target) const
{
    if (!IsAttackState())
        return false;
    if (auto t = target_.lock())
        return t->id_ == target->id_;
    return false;
}

bool AttackComp::IsAttackState() const
{
    return owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking;
}

void AttackComp::SetAttackState(bool value)
{
    if (IsAttackState() != value)
    {
        if (value)
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
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
    if (const auto t = target_.lock())
        return t->id_ == target->id_;
    return false;
}

}
}
