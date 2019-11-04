#include "stdafx.h"
#include "Fail.h"

namespace AI {

Fail::Fail(const ArgumentsType& arguments) :
    Decorator(arguments)
{ }

Node::Status Fail::Execute(Agent & agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return ReturnStatus(Node::Status::CanNotExecute);

    auto status = child_->Execute(agent, timeElapsed);
    if (status == Node::Status::Running)
        return ReturnStatus(Node::Status::Running);

    return ReturnStatus(Node::Status::Failed);
}

}
