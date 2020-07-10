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


#include "AiSortByDistance.h"
#include "../Game.h"
#include "../Npc.h"
#include <sa/Assert.h>
#include <CleanupNs.h>

namespace AI {
namespace Filters {

void SortByDistance::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    if (entities.size() == 0)
        return;

    Game::Npc& chr = GetNpc(agent);
    ASSERT(chr.HasGame());

    auto& game = *chr.GetGame();
    std::map<uint32_t, float> sorting;

    for (auto id : entities)
    {
        auto* object = game.GetObject<Game::GameObject>(id);
        sorting[id] = chr.GetDistance(object);
    }
    std::sort(entities.begin(), entities.end(), [&sorting](uint32_t i, uint32_t j)
    {
        return sorting[i] < sorting[j];
    });
}

}

}
