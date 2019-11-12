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

    Nodes::iterator nIt = agent.context_.Exists<Nodes::iterator>(id_) ? agent.context_.Get<Nodes::iterator>(id_) : children_.begin();

    while (nIt != children_.end())
    {
        // Call until one failed
        Status status = (*nIt)->Execute(agent, timeElapsed);
        if (status != Status::Finished)
            return status;
        ++nIt;
        agent.context_.Set(id_, nIt);
    }
    agent.context_.Delete<Nodes::iterator>(id_);
    return Status::Finished;
}

}
