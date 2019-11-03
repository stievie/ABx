#include "stdafx.h"
#include "TimedNode.h"

namespace AI {

TimedNode::TimedNode(const NodeFactoryContext& ctx) :
    Node(ctx),
    millis_(ctx.millis)
{
}

TimedNode::~TimedNode() = default;

Node::Status TimedNode::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    if (timer_ == NOT_STARTED)
    {
        timer_ = millis_;
        Status status = ExecuteStart(agent, timeElapsed);
        if (status == Status::Finished)
            timer_ = NOT_STARTED;
        return status;
    }

    if (timer_ - timeElapsed > 0)
    {
        timer_ += timeElapsed;
        Status status = ExecuteRunning(agent, timeElapsed);
        if (status == Status::Finished)
            timer_ = NOT_STARTED;
        return status;
    }

    timer_ = NOT_STARTED;
    return ExecuteExpired(agent, timeElapsed);
}

Node::Status TimedNode::ExecuteStart(Agent&, uint32_t)
{
    return Status::Running;
}

Node::Status TimedNode::ExecuteRunning(Agent&, uint32_t)
{
    return Status::Running;
}

Node::Status TimedNode::ExecuteExpired(Agent&, uint32_t)
{
    return Status::Finished;
}

}
