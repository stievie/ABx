#include "stdafx.h"
#include "Parallel.h"

namespace AI {

Parallel::Parallel(const NodeFactoryContext& ctx) :
    Composite(ctx)
{ }

Parallel::~Parallel() = default;

Node::Status Parallel::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    bool totalRunning = false;
    for (auto& child : children_)
    {
        const bool isRunning = child->Execute(agent, timeElapsed) == Node::Status::Running;
        totalRunning |= isRunning;
    }
    return totalRunning ? Node::Status::Running : Node::Status::Finished;
}

}
