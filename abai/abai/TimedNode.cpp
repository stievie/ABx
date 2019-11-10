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
        return Status::CanNotExecute;

    uint32_t timer = NOT_STARTED;
    auto it = agent.timers_.find(id_);
    if (it != agent.timers_.end())
        timer = (*it).second;

    if (timer == NOT_STARTED)
    {
        timer = millis_;
        Status status = ExecuteStart(agent, timeElapsed);
        if (status == Status::Finished)
            timer = NOT_STARTED;
        agent.timers_[id_] = timer;
        return status;
    }

    if (timer - timeElapsed > 0)
    {
        timer -= timeElapsed;
        Status status = ExecuteRunning(agent, timeElapsed);
        if (status == Status::Finished)
            timer = NOT_STARTED;
        agent.timers_[id_] = timer;
        return status;
    }

    agent.timers_[id_] = timer;
    return ExecuteExpired(agent, timeElapsed);
}

Node::Status TimedNode::ExecuteStart(Agent& agent, uint32_t)
{
    agent.runningActions_.emplace(id_);
    return Status::Running;
}

Node::Status TimedNode::ExecuteRunning(Agent&, uint32_t)
{
    return Status::Running;
}

Node::Status TimedNode::ExecuteExpired(Agent& agent, uint32_t)
{
    agent.runningActions_.erase(id_);
    return Status::Finished;
}

}
