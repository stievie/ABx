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
        return Status::CanNotExecute;
    auto status = DoAction(agent, timeElapsed);
    switch (status)
    {
    case Status::Running:
        agent.context_.runningActions_.emplace(id_);
        break;
    case Status::Finished:
    case Status::Failed:
        agent.context_.runningActions_.erase(id_);
        break;
    default:
        break;
    }
    return status;
}

}
