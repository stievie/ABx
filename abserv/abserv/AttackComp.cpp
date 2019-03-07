#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"

namespace Game {
namespace Components {

void AttackComp::Update(uint32_t /* timeElapsed */)
{
    // TODO: Attack speed, make damage etc.
}

void AttackComp::Cancel()
{
    attacking_ = false;
    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void AttackComp::Attack(std::shared_ptr<Actor> target)
{
    if (!target)
        return;
    target_ = target;
    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
    attacking_ = true;
    lastAttackTime_ = Utils::Tick();
}

}
}
