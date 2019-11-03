#include "stdafx.h"
#include "Inverter.h"

namespace AI {

Inverter::Inverter(const NodeFactoryContext& ctx) :
    Decorator(ctx)
{ }

Node::Status Inverter::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Decorator::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Node::Status::CanNotExecute;

    auto status = child_->Execute(agent, timeElapsed);
    switch (status)
    {
    case Status::CanNotExecute:
        return Status::Finished;
    case Status::Finished:
        return Status::Failed;
    case Status::Failed:
        return Status::Finished;
    default:
        return Status::Running;
    }
}

}
