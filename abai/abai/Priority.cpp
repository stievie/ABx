#include "stdafx.h"
#include "Priority.h"

namespace AI {

Priority::Priority(const ArgumentsType& arguments) :
    Composite(arguments)
{ }

Priority::~Priority() = default;

Node::Status Priority::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    for (auto child : children_)
    {
        Status status = child->Execute(agent, timeElapsed);
        if (status != Status::Failed && status != Status::CanNotExecute)
            return status;
        // If failed try the next
    }
    return Status::Failed;
}

}
