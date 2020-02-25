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

#pragma once

#include <stdint.h>
#include <unordered_map>
#include <memory>
#include <sa/Iteration.h>
#include "AiDefines.h"

namespace AI {

class Agent;

// In ABx we don't use a Zone, the NPC should call Agent::Update()
class Zone
{
private:
    std::unordered_map<Id, std::shared_ptr<Agent>> agents_;
public:
    Zone(const std::string& name);
    ~Zone();

    bool AddAgent(std::shared_ptr<Agent> agent);
    bool RemoveAgent(std::shared_ptr<Agent> agent);

    void Update(uint32_t timeElapsed);

    std::string name_;

    template<typename Callback>
    inline void VisitAgents(Callback&& callback);
};

template<typename Callback>
inline void Zone::VisitAgents(Callback&& callback)
{
    for (auto agent : agents_)
    {
        if (callback(*agent.second) != Iteration::Continue)
            break;
    }
}

}
