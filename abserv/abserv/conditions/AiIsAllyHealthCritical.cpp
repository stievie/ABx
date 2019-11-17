#include "stdafx.h"
#include "AiIsAllyHealthCritical.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsAllyHealthCritical::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    bool result = false;
    npc.VisitAlliesInRange(Game::Ranges::Aggro, [&result](const Game::Actor* o)
    {
        if (result)
            return;
        if (!o->IsDead() && o->resourceComp_->GetHealth() < CRITICAL_HP_THRESHOLD)
            result = true;
    });

    return result;
}

}
}
