#include "stdafx.h"
#include "AttackComp.h"
#include "Actor.h"

namespace Game {
namespace Components {

void AttackComp::Update(uint32_t /* timeElapsed */)
{
}

void AttackComp::Cancel()
{
    attacking_ = false;
    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

}
}
