#include "stdafx.h"
#include "AiFlee.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {
namespace Actions {

Node::Status Flee::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    // TODO: See who is damaging us and run away from that position
    (void)npc;
    return Status::Failed;
}

}
}
