#include "stdafx.h"
#include "AiIsSelfHealthCritical.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsSelfHealthCritical::Evaluate(Agent& agent)
{
    auto& npc = AI::GetNpc(agent);
    if (npc.IsDead())
        // Too late
        return false;
    return npc.resourceComp_->GetHealthRatio() < CRITICAL_HP_THRESHOLD;
}

}
}
