#include "stdafx.h"
#include "Root.h"

namespace AI {

Root::Root(const NodeFactoryContext& ctx) :
    Decorator(ctx)
{
}

Node::Status Root::Execute(Agent& agent, uint32_t timeElapsed)
{
    return child_->Execute(agent, timeElapsed);
}

}