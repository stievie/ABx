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
    if (home.Equals(npc.GetPosition()))
        return Status::Finished;
    return Status::Running;
}

GoHome::GoHome(const ArgumentsType& arguments) :
    Action(arguments)
{
}

}
}
