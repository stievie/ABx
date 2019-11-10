#include "stdafx.h"
#include "Action.h"
#include "Agent.h"

namespace AI {

Action::Action(const ArgumentsType& arguments) :
    Node(arguments)
{ }

Action::~Action() = default;

Node::Status Action::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return ReturnStatus(Status::CanNotExecute);
    auto status = DoAction(agent, timeElapsed);
    switch (status)
    {
    case Status::Running:
        agent.runningActions_.emplace(id_);
        break;
    case Status::Finished:
    case Status::Failed:
        agent.runningActions_.erase(id_);
        break;
    default:
        break;
    }
    return ReturnStatus(status);
}

}
