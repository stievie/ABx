#include "stdafx.h"
#include "Limit.h"
#include "Agent.h"

namespace AI {

Limit::Limit(const ArgumentsType& arguments) :
    Decorator(arguments)
{
    if (arguments.size() != 0)
        limit_ = static_cast<uint32_t>(atoi(arguments[0].c_str()));
}

Node::Status Limit::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return Node::Status::CanNotExecute;

    size_t executions = 0;
    if (agent.context_.Has<limit_type>(id_))
        executions = agent.context_.Get<limit_type>(id_);

    if (executions >= limit_)
        return Node::Status::Finished;

    auto status = child_->Execute(agent, timeElapsed);

    agent.context_.Set<limit_type>(id_, executions + 1);
    if (status == Node::Status::Running)
        return Node::Status::Running;

    return Node::Status::Failed;
}

}
