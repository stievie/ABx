#include "stdafx.h"
#include "TimedNode.h"
#include "Agent.h"

namespace AI {

TimedNode::TimedNode(const ArgumentsType& arguments) :
    Node(arguments)
{
    if (arguments.size() != 0)
        millis_ = atoi(arguments[0].c_str());
}

TimedNode::~TimedNode() = default;

Node::Status TimedNode::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return ReturnStatus(Status::CanNotExecute);

    if (timer_ == NOT_STARTED)
    {
        timer_ = millis_;
        Status status = ExecuteStart(agent, timeElapsed);
        if (status == Status::Finished)
            timer_ = NOT_STARTED;
        return ReturnStatus(status);
    }

    if (timer_ - timeElapsed > 0)
    {
        timer_ -= timeElapsed;
        Status status = ExecuteRunning(agent, timeElapsed);
        if (status == Status::Finished)
            timer_ = NOT_STARTED;
        return ReturnStatus(status);
    }

    timer_ = NOT_STARTED;
    return ReturnStatus(ExecuteExpired(agent, timeElapsed));
}

Node::Status TimedNode::ExecuteStart(Agent& agent, uint32_t)
{
    agent.runningActions_.emplace(id_);
    return ReturnStatus(Status::Running);
}

Node::Status TimedNode::ExecuteRunning(Agent&, uint32_t)
{
    return ReturnStatus(Status::Running);
}

Node::Status TimedNode::ExecuteExpired(Agent& agent, uint32_t)
{
    agent.runningActions_.erase(id_);
    return ReturnStatus(Status::Finished);
}

}
