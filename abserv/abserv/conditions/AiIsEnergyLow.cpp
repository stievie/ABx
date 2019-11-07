#include "stdafx.h"
#include "AiIsEnergyLow.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsEnergyLow::Evaluate(Agent& agent)
{
    auto& npc = AI::GetNpc(agent);
    if (npc.IsDead())
        // Too late
        return false;
    return npc.resourceComp_->GetEnergyRatio() <= LOW_ENERGY_THRESHOLD;
}

}
}
