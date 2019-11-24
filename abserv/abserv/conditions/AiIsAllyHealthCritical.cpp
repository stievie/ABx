#include "stdafx.h"
#include "AiIsAllyHealthCritical.h"
#include "../Game.h"
#include "../Npc.h"
#include "Subsystems.h"
#include "Random.h"

namespace AI {
namespace Conditions {

bool IsAllyHealthCritical::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    bool result = false;
    auto* rnd = GetSubsystem<Crypto::Random>();

    npc.VisitAlliesInRange(Game::Ranges::HalfCompass, [&result, &rnd](const Game::Actor& o)
    {
        if (!o.IsDead() && rnd->Matches(CRITICAL_HP_THRESHOLD, o.resourceComp_->GetHealth(), 10))
        {
            result = true;
            return Iteration::Break;
        }
        return Iteration::Continue;
    });

    return result;
}

}
}
