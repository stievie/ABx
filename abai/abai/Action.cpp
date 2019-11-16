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
    {
        if (auto ca = agent.context_.currentAction_.lock())
        {
            if (ca->GetId() == id_)
                agent.context_.currentAction_.reset();
        }
        return Status::CanNotExecute;
    }
    const auto status = DoAction(agent, timeElapsed);
    switch (status)
    {
    case Status::Running:
        agent.context_.currentAction_ = shared_from_this();
        break;
    case Status::Finished:
    case Status::Failed:
        if (auto ca = agent.context_.currentAction_.lock())
        {
            if (ca->GetId() == id_)
                agent.context_.currentAction_.reset();
        }
        break;
    default:
        break;
    }
    return status;
}

}
