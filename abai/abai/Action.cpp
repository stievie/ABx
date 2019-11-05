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
        agent.currentAction_ = id_;
        break;
    case Status::Finished:
        if (agent.currentAction_ == id_)
            agent.currentAction_ = INVALID_ID;
        break;
    default:
        break;
    }
    return ReturnStatus(status);
}

}
