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
#include "AiSelectRandom.h"

namespace AI {
namespace Filters {

SelectRandom::SelectRandom(const ArgumentsType& arguments) :
    Filter(arguments)
{
    if (arguments.size() != 0)
        count_ = static_cast<uint32_t>(atoi(arguments[0].c_str()));
    else
        count_ = 1;
}

void SelectRandom::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    if (count_ == 0)
        return;
    if (count_ >= entities.size())
        return;

    auto* rnd = GetSubsystem<Crypto::Random>();
    AI::AgentIds copy(entities);
    entities.clear();
    while (entities.size() < count_ && copy.size() > 0)
    {
        const float rn = rnd->GetFloat();
        auto it = Utils::SelectRandomly(copy.begin(), copy.end(), rn);
        entities.push_back(*it);
        copy.erase(it);
    }
}

}
}
