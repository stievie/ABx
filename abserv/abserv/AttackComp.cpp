#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"
#include "Subsystems.h"
#include "Random.h"

namespace Game {
namespace Components {

void AttackComp::Update(uint32_t /* timeElapsed */)
{
    if (IsAttackState())
    {
        if (!attacking_)
        {
            // New attack
            attackSpeed_ = owner_.GetAttackSpeed();
            interrupted_ = false;
            if (Utils::TimePassed(lastAttackTime_) >= attackSpeed_ / 2)
            {
                lastAttackTime_ = Utils::Tick();
                attacking_ = true;
                damageType_ = owner_.GetAttackDamageType();
            }
        }
        else
        {
            // Now we are really attacking. This can be interrupted.
            if (Utils::TimePassed(lastAttackTime_) >= attackSpeed_)
            {
                // Done attack -> apply damage
                attacking_ = false;
                if (interrupted_)
                {
                    lastError_ = AB::GameProtocol::AttackErrorInterrupted;
                    owner_.OnInterruptedAttack();
                }
                else
                {
                    if (auto t = target_.lock())
                    {
                        const float criticalChance = owner_.GetCriticalChance(t.get());
                        auto rnd = GetSubsystem<Crypto::Random>();
                        bool critical = criticalChance >= rnd->GetFloat();
                        // Critical hit -> always weapons max damage
                        int32_t damage = owner_.GetAttackDamage(critical);
                        // Source effects may modify the damage
                        owner_.effectsComp_->GetDamage(damageType_, damage, critical);
                        if (t->OnAttacked(&owner_, damageType_, damage))
                        {
                            // Some effects may prevent attacks, e.g. blocking
                            if (critical)
                                // Some effect may prevent critical hits
                                critical = t->OnGetCriticalHit(&owner_);
                            if (critical)
                                damage = static_cast<int>(static_cast<float>(damage) * std::sqrt(2.0f));
                            t->ApplyDamage(&owner_, 0, damageType_, damage, owner_.GetArmorPenetration());
                        }
                        else
                        {
                            lastError_ = AB::GameProtocol::AttackErrorInterrupted;
                            owner_.OnInterruptedAttack();
                        }
                    }
                }
            }
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
    SetAttackState(false);
}

void AttackComp::Attack(std::shared_ptr<Actor> target)
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
    SetAttackState(true);
    lastAttackTime_ = 0;
}

bool AttackComp::IsAttackState() const
{
    return owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking;
}

void AttackComp::SetAttackState(bool value)
{
    if (IsAttackState())
    {
        if (value)
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
        else
            owner_.stateComp_.Reset();
    }
}

bool AttackComp::Interrupt()
{
    if (attacking_)
    {
        interrupted_ = true;
        return true;
    }
    return false;
}

}
}
