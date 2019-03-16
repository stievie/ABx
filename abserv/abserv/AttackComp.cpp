#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"

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
            if (Utils::TimePassed(lastAttackTime_) >= attackSpeed_ / 2)
            {
                lastAttackTime_ = Utils::Tick();
                attacking_ = true;
                damageType_ = owner_.GetAttackDamageType();
                baseDamage_ = owner_.GetAttackDamage();
            }
        }
        else
        {
            // Now we are really attacking. This can be interrupted.
            if (Utils::TimePassed(lastAttackTime_) >= attackSpeed_)
            {
                // Done attack -> apply damage
                attacking_ = false;
                if (auto t = target_.lock())
                {
                    int32_t damage = baseDamage_;
                    owner_.effectsComp_.GetDamage(damageType_, damage);
                    if (t->OnAttacked(&owner_, damageType_, damage))
                        t->damageComp_.ApplyDamage(damageType_, damage);
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
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
    }
}

}
}
