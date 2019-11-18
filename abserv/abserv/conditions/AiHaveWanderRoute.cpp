#include "stdafx.h"
#include "AiHaveWanderRoute.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool HaveWanderRoute::Evaluate(Agent& agent, const Node&)
{
    const auto& npc = GetNpc(agent);
    if (!npc.IsWander())
        return false;

    return npc.wanderComp_->HaveRoute();
}

}

}
