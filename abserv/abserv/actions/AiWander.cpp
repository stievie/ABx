#include "stdafx.h"
#include "AiWander.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {
namespace Actions {

Node::Status Wander::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (!npc.IsWander())
        return Status::Failed;

    if (npc.wanderComp_->IsWandering())
        return Status::Running;

    if (!npc.wanderComp_->Wander(true))
        return Status::Failed;

    return Status::Running;
}

Wander::Wander(const ArgumentsType& arguments) :
    Action(arguments)
{ }

}
}
