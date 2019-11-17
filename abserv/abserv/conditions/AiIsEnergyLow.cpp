#include "stdafx.h"
#include "AiIsEnergyLow.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsEnergyLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    // TODO: Also consider what Skills the NPC has, e.g. only skills that
    // need a lot of energy.
    return npc.resourceComp_->GetEnergyRatio() <= LOW_ENERGY_THRESHOLD;
}

}
}
