#include "stdafx.h"
#include "AiGoHome.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "../Mechanic.h"
#include "Logger.h"

//#define DEBUG_AI

namespace AI {
namespace Actions {

Node::Status GoHome::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    const Math::Vector3& home = npc.GetHomePos();
    if (home.Equals(Math::Vector3::Zero))
        return Status::Failed;

    if (home.Equals(npc.GetPosition(), Game::AT_POSITON_THRESHOLD * 2.0f))
        return Status::Finished;

    if (IsCurrentAction(agent))
        return Status::Running;

    if (npc.GotoHomePos())
    {
#ifdef DEBUG_AI
        LOG_DEBUG << npc.GetName() << " goes home from " << npc.GetPosition().ToString() <<
                     " to " << home.ToString() << std::endl;
#endif
        return Status::Running;
    }
    return Status::Finished;
}

GoHome::GoHome(const ArgumentsType& arguments) :
    Action(arguments)
{ }

}
}
