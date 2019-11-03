#include "stdafx.h"
#include "Priority.h"

namespace AI {

Priority::Priority(const NodeFactoryContext& ctx) :
    Composite(ctx)
{ }

Priority::~Priority() = default;

void Priority::Initialize()
{
    Composite::Initialize();
    it_ = children_.begin();
}

Node::Status Priority::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    while (it_ != children_.end())
    {
        Status status = (*it_)->Execute(agent, timeElapsed);
        if (status != Status::Failed)
            return status;
        it_++;
    }
    return Status::Failed;
}

}
