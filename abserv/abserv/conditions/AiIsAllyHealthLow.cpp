#include "stdafx.h"
#include "AiIsAllyHealthLow.h"
#include "../Game.h"
#include "../Npc.h"
#include "Random.h"
#include "Subsystems.h"

namespace AI {
namespace Conditions {

bool IsAllyHealthLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    bool result = false;
    auto* rnd = GetSubsystem<Crypto::Random>();
    npc.VisitAlliesInRange(Game::Ranges::HalfCompass, [&result, &rnd](const Game::Actor& o)
    {
        if (!o.IsDead() && rnd->Matches(LOW_HP_THRESHOLD, o.resourceComp_->GetHealthRatio(), 10.0f))
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
