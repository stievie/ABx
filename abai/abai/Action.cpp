#include "stdafx.h"
#include "Action.h"
#include "Agent.h"

namespace AI {

Action::Action(const ArgumentsType& arguments) :
    Node(arguments)
{ }

Action::~Action() = default;

bool Action::IsCurrentAction(const Agent& agent) const
{
    if (auto co = agent.context_.currentAction_.lock())
        return co->GetId() == id_;
    return false;
}

Node::Status Action::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
    {
        if (IsCurrentAction(agent))
            agent.context_.currentAction_.reset();
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
        if (IsCurrentAction(agent))
            agent.context_.currentAction_.reset();
        break;
    default:
        break;
    }
    return status;
}

}
