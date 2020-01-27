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
#include "Zone.h"
#include "Agent.h"
#include "Node.h"

namespace AI {

Zone::Zone(const std::string& name) :
    name_(name)
{ }

Zone::~Zone() = default;

bool Zone::AddAgent(std::shared_ptr<Agent> agent)
{
    if (!agent)
        return false;
    agents_.emplace(agent->GetId(), agent);
    agent->SetZone(this);
    return true;
}

bool Zone::RemoveAgent(std::shared_ptr<Agent> agent)
{
    if (!agent)
        return false;

    if (agent->GetZone() == this)
        agent->SetZone(nullptr);
    return agents_.erase(agent->GetId()) != 0;
}

void Zone::Update(uint32_t timeElapsed)
{
    for (auto& agent : agents_)
    {
        if (agent.second->pause_)
            continue;

        agent.second->Update(timeElapsed);
    }
}

}
