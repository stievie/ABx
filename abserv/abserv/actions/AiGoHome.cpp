#include "stdafx.h"
#include "AiGoHome.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {
namespace Actions {

Node::Status GoHome::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    Math::Vector3 home = npc.GetHomePos();
    if (home.Equals(Math::Vector3::Zero))
        return Status::Failed;
    if (agent.IsActionRunning(id_))
    {
        if (home.Distance(npc.GetPosition()) <= 0.3f)
            return Status::Finished;
    }
    if (npc.GotoHomePos())
        return Status::Running;
    return Status::Failed;
}

GoHome::GoHome(const ArgumentsType& arguments) :
    Action(arguments)
{
}

}
}
