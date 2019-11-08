#include "stdafx.h"
#include "Sequence.h"

namespace AI {

Sequence::Sequence(const ArgumentsType& arguments) :
    Composite(arguments)
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
        return ReturnStatus(Status::CanNotExecute);

    while (it_ != children_.end())
    {
        // Call until one failed
        Status status = (*it_)->Execute(agent, timeElapsed);
        if (status != Status::Finished)
            return ReturnStatus(status);
        ++it_;
    }
    return ReturnStatus(Status::Finished);
}

}
