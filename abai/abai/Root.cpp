#include "stdafx.h"
#include "Root.h"

namespace AI {

Root::Root() :
    Decorator(ArgumentsType{})
{
}

Node::Status Root::Execute(Agent& agent, uint32_t timeElapsed)
{
    return ReturnStatus(child_->Execute(agent, timeElapsed));
}

}