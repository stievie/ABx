#include "stdafx.h"
#include "Parallel.h"

namespace AI {

Parallel::Parallel(const ArgumentsType& arguments) :
    Composite(arguments)
{ }

Parallel::~Parallel() = default;

Node::Status Parallel::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return ReturnStatus(Status::CanNotExecute);

    bool totalRunning = false;
    for (auto& child : children_)
    {
        const bool isRunning = child->Execute(agent, timeElapsed) == Status::Running;
        totalRunning |= isRunning;
    }
    return totalRunning ? ReturnStatus(Status::Running) : ReturnStatus(Status::Finished);
}

}
