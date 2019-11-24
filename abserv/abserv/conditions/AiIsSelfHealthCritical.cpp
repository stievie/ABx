#include "stdafx.h"
#include "AiIsSelfHealthCritical.h"
#include "../Game.h"
#include "../Npc.h"
#include "Subsystems.h"
#include "Random.h"

namespace AI {
namespace Conditions {

bool IsSelfHealthCritical::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    if (npc.IsDead())
        // Too late
        return false;
    auto* rnd = GetSubsystem<Crypto::Random>();
    return rnd->Matches(CRITICAL_HP_THRESHOLD, npc.resourceComp_->GetHealth(), 10);
}

}
}
