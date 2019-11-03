#include "stdafx.h"
#include "Succeed.h"

namespace AI {

Succeed::Succeed(const NodeFactoryContext& ctx) :
    Decorator(ctx)
{
}

Node::Status Succeed::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return Node::Status::CanNotExecute;

    auto status = child_->Execute(agent, timeElapsed);
    if (status == Node::Status::Running)
        return Node::Status::Running;

    return Node::Status::Finished;
}

}
