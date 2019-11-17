#include "stdafx.h"
#include "AiIsAllyHealthLow.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsAllyHealthLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    bool result = false;
    npc.VisitAlliesInRange(Game::Ranges::Aggro, [&result](const Game::Actor* o)
    {
        if (result)
            return;
        if (!o->IsDead() && o->resourceComp_->GetHealthRatio() < LOW_HP_THRESHOLD)
            result = true;
    });

    return result;
}

}
}
