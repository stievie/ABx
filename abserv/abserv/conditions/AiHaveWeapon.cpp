#include "stdafx.h"
#include "AiHaveWeapon.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool HaveWeapon::Evaluate(Agent& agent, const Node&)
{
    const auto& npc = GetNpc(agent);
    auto* weapon = npc.GetWeapon();
    return !!weapon;
}

}

}
