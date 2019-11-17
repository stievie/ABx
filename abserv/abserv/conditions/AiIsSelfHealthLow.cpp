#include "stdafx.h"
#include "AiIsSelfHealthLow.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsSelfHealthLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    if (npc.IsDead())
        // Too late
        return false;
    return npc.resourceComp_->GetHealthRatio() < LOW_HP_THRESHOLD;
}

}
}
