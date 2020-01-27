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
#include "Inverter.h"

namespace AI {

Inverter::Inverter(const ArgumentsType& arguments) :
    Decorator(arguments)
{ }

Node::Status Inverter::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Decorator::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;

    auto status = child_->Execute(agent, timeElapsed);
    switch (status)
    {
    case Status::CanNotExecute:
        return Status::Finished;
    case Status::Finished:
        return Status::Failed;
    case Status::Failed:
        return Status::Finished;
    default:
        return Status::Running;
    }
}

}
