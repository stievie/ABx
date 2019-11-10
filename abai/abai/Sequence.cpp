#include "stdafx.h"
#include "Sequence.h"
#include "Agent.h"

namespace AI {

Sequence::Sequence(const ArgumentsType& arguments) :
    Composite(arguments)
{ }

Sequence::~Sequence() = default;

Node::Status Sequence::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    const auto it = agent.iterators_.find(id_);
    Nodes::iterator nIt = (it != agent.iterators_.end()) ? (*it).second : children_.begin();

    while (nIt != children_.end())
    {
        // Call until one failed
        Status status = (*nIt)->Execute(agent, timeElapsed);
        if (status != Status::Finished)
            return status;
        ++nIt;
        agent.iterators_.emplace(id_, nIt);
    }
    agent.iterators_.erase(id_);
    return Status::Finished;
}

}
