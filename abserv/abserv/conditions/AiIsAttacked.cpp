#include "stdafx.h"
#include "AiIsAttacked.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsAttacked::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);

    if (Utils::TimeElapsed(npc.damageComp_->lastDamage_) < 1000)
    {
        if (auto attacker = npc.damageComp_->GetLastDamager())
        {
            if (attacker->IsDead())
                return false;
            return true;
        }
    }
    return false;
}

}
}
