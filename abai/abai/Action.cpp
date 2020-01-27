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
#include "Action.h"
#include "Agent.h"

namespace AI {

Action::Action(const ArgumentsType& arguments) :
    Node(arguments)
{ }

Action::~Action() = default;

bool Action::IsCurrentAction(const Agent& agent) const
{
    if (auto co = agent.context_.currentAction_.lock())
        return co->GetId() == id_;
    return false;
}

void Action::SetCurrentAction(Agent& agent)
{
    if (!IsCurrentAction(agent))
        agent.context_.currentAction_ = shared_from_this();
}

void Action::UnsetCurrentAction(Agent& agent)
{
    if (IsCurrentAction(agent))
        agent.context_.currentAction_.reset();
}

Node::Status Action::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
    {
        UnsetCurrentAction(agent);
        return Status::CanNotExecute;
    }
    const auto status = DoAction(agent, timeElapsed);
    switch (status)
    {
    case Status::Running:
        SetCurrentAction(agent);
        break;
    case Status::Finished:
    case Status::Failed:
        UnsetCurrentAction(agent);
        break;
    default:
        break;
    }
    return status;
}

}
