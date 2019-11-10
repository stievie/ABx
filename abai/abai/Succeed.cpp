#include "stdafx.h"
#include "Succeed.h"

namespace AI {

Succeed::Succeed(const ArgumentsType& arguments) :
    Decorator(arguments)
{ }

Node::Status Succeed::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    auto status = child_->Execute(agent, timeElapsed);
    if (status == Status::Running)
        return Status::Running;

    return Status::Finished;
}

}
