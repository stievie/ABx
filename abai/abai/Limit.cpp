#include "stdafx.h"
#include "Limit.h"
#include "Agent.h"

namespace AI {

Limit::Limit(const NodeFactoryContext& ctx) :
    Decorator(ctx),
    limit_(ctx.times)
{ }

Node::Status Limit::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return Node::Status::CanNotExecute;

    const auto it = agent.limits_.find(GetId());
    const size_t executions = (it != agent.limits_.end()) ? (*it).second : 0;

    if (executions >= limit_)
        return Node::Status::Finished;

    auto status = child_->Execute(agent, timeElapsed);

    agent.limits_[GetId()] = executions + 1;
    if (status == Node::Status::Running)
        return Node::Status::Running;

    return Node::Status::Failed;
}

}