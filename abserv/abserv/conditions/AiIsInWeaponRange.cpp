#include "stdafx.h"
#include "AiIsInWeaponRange.h"
#include "../AiAgent.h"
#include "../Npc.h"
#include "../Item.h"
#include "../Game.h"
#include "../Mechanic.h"

namespace AI {
namespace Conditions {

bool IsInWeaponRange::Evaluate(Agent& agent, const Node&)
{
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return true;

    const auto& npc = GetNpc(agent);
    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return true;

    auto* weapon = npc.GetWeapon();
    if (!weapon)
        return npc.IsInRange(Game::Ranges::Touch, target.get());

    float dist = weapon->GetWeaponRange();
    return (npc.GetDistance(target.get()) <= dist);
}

}

}
