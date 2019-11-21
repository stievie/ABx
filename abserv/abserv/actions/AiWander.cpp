#include "stdafx.h"
#include "AiWander.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "Logger.h"

//#define DEBUG_AI

namespace AI {
namespace Actions {

Node::Status Wander::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (!npc.IsWander())
    {
#ifdef DEBUG_AI
        LOG_WARNING << npc.GetName() << " has no wander component" << std::endl;
#endif
        return Status::Failed;
    }
    if (npc.IsImmobilized())
        return Status::Failed;

    if (npc.wanderComp_->IsWandering())
        return Status::Running;

    if (!npc.wanderComp_->Wander(true))
    {
#ifdef DEBUG_AI
        LOG_DEBUG << npc.GetName() << " failed wandering" << std::endl;
#endif
        return Status::Failed;
    }

#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " start wandering" << std::endl;
#endif
    return Status::Running;
}

Wander::Wander(const ArgumentsType& arguments) :
    Action(arguments)
{ }

}
}
