#include "stdafx.h"
#include "Sequence.h"

namespace AI {

Sequence::Sequence(const NodeFactoryContext& ctx) :
    Composite(ctx)
{ }

Sequence::~Sequence() = default;

void Sequence::Initialize()
{
    Composite::Initialize();
    it_ = children_.begin();
}

Node::Status Sequence::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    while (it_ != children_.end())
    {
        Status status = (*it_)->Execute(agent, timeElapsed);
        if (status != Status::Finished)
            return status;
        it_++;
    }
    return Status::Finished;
}

}
