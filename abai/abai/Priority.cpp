#include "stdafx.h"
#include "Priority.h"

namespace AI {

Priority::Priority(const ArgumentsType& arguments) :
    Composite(arguments)
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
        return ReturnStatus(Status::CanNotExecute);

    while (it_ != children_.end())
    {
        Status status = (*it_)->Execute(agent, timeElapsed);
        if (status != Status::Failed)
            return ReturnStatus(status);
        ++it_;
    }
    return ReturnStatus(Status::Failed);
}

}
