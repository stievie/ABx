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
        return ReturnValue(agent, Status::CanNotExecute);

    Nodes::iterator nIt = agent.context_.Has<Nodes::iterator>(id_) ? agent.context_.Get<Nodes::iterator>(id_) : children_.begin();

    while (nIt != children_.end())
    {
        // Call until one failed
        Status status = (*nIt)->Execute(agent, timeElapsed);
        if (status != Status::Finished)
            return ReturnValue(agent, status);
        ++nIt;
        agent.context_.Set(id_, nIt);
    }
    agent.context_.Delete<Nodes::iterator>(id_);
    return ReturnValue(agent, Status::Finished);
}

}
