/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Agent.h"
#include "TimedNode.h"

namespace AI {

TimedNode::TimedNode(const ArgumentsType& arguments) :
    Action(arguments)
{
    GetArgument<uint32_t>(arguments, 0, millis_);
}

TimedNode::~TimedNode() = default;

Node::Status TimedNode::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return ReturnStatus(agent, Status::CanNotExecute);

    uint32_t timer = NOT_STARTED;
    if (agent.context_.Has<TimerType>(id_))
        timer = agent.context_.Get<TimerType>(id_);

    if (timer == NOT_STARTED)
    {
        timer = millis_;
        Status status = ExecuteStart(agent, timeElapsed);
        if (status == Status::Finished)
            timer = NOT_STARTED;
        agent.context_.Set<TimerType>(id_, timer);
        return ReturnStatus(agent, status);
    }

    if (timer - timeElapsed > 0)
    {
        timer -= timeElapsed;
        Status status = ExecuteRunning(agent, timeElapsed);
        if (status == Status::Finished)
            timer = NOT_STARTED;
        agent.context_.Set<TimerType>(id_, timer);
        return ReturnStatus(agent, status);
    }

    agent.context_.Set<TimerType>(id_, timer);
    return ReturnStatus(agent, ExecuteExpired(agent, timeElapsed));
}

Node::Status TimedNode::ExecuteStart(Agent& agent, uint32_t)
{
    SetCurrentAction(agent);
    return Status::Running;
}

Node::Status TimedNode::ExecuteRunning(Agent&, uint32_t)
{
    return Status::Running;
}

Node::Status TimedNode::ExecuteExpired(Agent& agent, uint32_t)
{
    UnsetCurrentAction(agent);
    return Status::Finished;
}

}
