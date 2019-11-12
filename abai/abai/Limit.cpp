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

    const size_t executions = (agent.context_.Exists<size_t>(id_)) ? agent.context_.Get<size_t>(id_) : 0;

    if (executions >= limit_)
        return Node::Status::Finished;

    auto status = child_->Execute(agent, timeElapsed);

    agent.context_.Set(id_, executions + 1);
    if (status == Node::Status::Running)
        return Node::Status::Running;

    return Node::Status::Failed;
}

}
