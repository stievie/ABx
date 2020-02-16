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
        return ReturnStatus(agent, Node::Status::CanNotExecute);

    size_t executions = 0;
    if (agent.context_.Has<limit_type>(id_))
        executions = agent.context_.Get<limit_type>(id_);

    if (executions >= limit_)
        return ReturnStatus(agent, Node::Status::Finished);

    auto status = child_->Execute(agent, timeElapsed);

    agent.context_.Set<limit_type>(id_, executions + 1);
    if (status == Node::Status::Running)
        return ReturnStatus(agent, Node::Status::Running);

    return ReturnStatus(agent, Node::Status::Failed);
}

}
