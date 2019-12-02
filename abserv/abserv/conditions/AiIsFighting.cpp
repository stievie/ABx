#include "stdafx.h"
#include "AiIsFighting.h"
#include "../Npc.h"
#include "../Crowd.h"
#include "Utils.h"

namespace AI {
namespace Conditions {

bool IsFighting::Evaluate(Agent& agent, const Node&)
{
    auto& npc = GetNpc(agent);
    Game::Group* crowd = npc.GetGroup();
    if (!crowd)
    {
        return (Utils::TimeElapsed(npc.damageComp_->lastDamage_) < 1000) ||
            npc.attackComp_->IsAttackState();
    }

    bool result = false;
    crowd->VisitMembers([&result](const Game::Actor& current)
    {
        result = (Utils::TimeElapsed(current.damageComp_->lastDamage_) < 1000) ||
            current.attackComp_->IsAttackState();
        if (result)
            return Iteration::Break;
        return Iteration::Continue;
    });
    return result;
}

}

}
