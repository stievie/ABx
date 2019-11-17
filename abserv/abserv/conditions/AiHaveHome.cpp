#include "stdafx.h"
#include "AiHaveHome.h"
#include "../Npc.h"
#include "Vector3.h"

namespace AI {
namespace Conditions {

bool HaveHome::Evaluate(Agent& agent, const Node&)
{
    const auto& npc = GetNpc(agent);
    return !npc.GetHomePos().Equals(Math::Vector3::Zero);
}

}
}
