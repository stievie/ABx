#include "stdafx.h"
#include "Root.h"

namespace AI {

Root::Root() :
    Decorator(ArgumentsType{})
{
    type_ = "Root";
}

Node::Status Root::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (!child_)
        return ReturnStatus(Status::CanNotExecute);
    return ReturnStatus(child_->Execute(agent, timeElapsed));
}

}