#include "stdafx.h"
#include "AiIsSelfHealthLow.h"
#include "../Game.h"
#include "../Npc.h"
#include "Subsystems.h"
#include "Random.h"

namespace AI {
namespace Conditions {

bool IsSelfHealthLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    if (npc.IsDead())
        // Too late
        return false;
    auto* rnd = GetSubsystem<Crypto::Random>();
    return rnd->Matches(LOW_HP_THRESHOLD, npc.resourceComp_->GetHealthRatio(), 10.0f);
}

}
}
