#include "stdafx.h"
#include "Inverter.h"

namespace AI {

Inverter::Inverter(const ArgumentsType& arguments) :
    Decorator(arguments)
{ }

Node::Status Inverter::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Decorator::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return ReturnStatus(Status::CanNotExecute);

    auto status = child_->Execute(agent, timeElapsed);
    switch (status)
    {
    case Status::CanNotExecute:
        return ReturnStatus(Status::Finished);
    case Status::Finished:
        return ReturnStatus(Status::Failed);
    case Status::Failed:
        return ReturnStatus(Status::Finished);
    default:
        return ReturnStatus(Status::Running);
    }
}

}
