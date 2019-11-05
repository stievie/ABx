#include "stdafx.h"
#include "AiSay.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {

Node::Status Say::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    (void)npc;
    return Status::Failed;
}

}
