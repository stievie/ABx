#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"

namespace Game {
namespace Components {

void AttackComp::Update(uint32_t /* timeElapsed */)
{
    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
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
                    t->damageComp_.ApplyDamage(damageType_, damage);
                }
            }
        }
    }
}

void AttackComp::Cancel()
{
    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void AttackComp::Attack(std::shared_ptr<Actor> target)
{
    if (!owner_.CanAttack())
        return;
    if (!target)
        // Attack needs a target
        return;
    if (target->IsUndestroyable() && target->CanBeAttacked())
        // Can not attack an destroyable target
        return;

    target_ = target;
    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
    lastAttackTime_ = 0;
}

}
}
