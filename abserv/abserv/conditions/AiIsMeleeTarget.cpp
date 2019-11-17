#include "stdafx.h"
#include "AiIsMeleeTarget.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsMeleeTarget::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);

    if (npc.damageComp_->IsGettingMeleeDamage())
    {
        if (auto attacker = npc.damageComp_->GetLastMeleeDamager())
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
