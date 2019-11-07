#include "stdafx.h"
#include "AiDie.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {
namespace Actions {

Node::Status Die::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (npc.Die())
        return Status::Finished;
    return Status::Failed;
}

Die::Die(const ArgumentsType& arguments) :
    Action(arguments)
{
}

}
}
