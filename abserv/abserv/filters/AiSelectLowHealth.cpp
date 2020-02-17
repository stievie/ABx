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
#include "AiSelectLowHealth.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

SelectLowHealth::SelectLowHealth(const ArgumentsType& arguments) :
    Filter(arguments)
{ }

void SelectLowHealth::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    std::map<uint32_t, std::pair<float, float>> sorting;
    auto* rnd = GetSubsystem<Crypto::Random>();

    chr.VisitAlliesInRange(Game::Ranges::HalfCompass, [&](const Game::Actor& o)
    {
        if (!o.IsDead() && rnd->Matches(LOW_HP_THRESHOLD, o.resourceComp_->GetHealthRatio(), 10.0f))
        {
            entities.push_back(o.id_);
            sorting[o.id_] = std::make_pair<float, float>(o.resourceComp_->GetHealthRatio(), o.GetDistance(&chr));
        }
        return Iteration::Continue;
    });
    std::sort(entities.begin(), entities.end(), [&sorting](uint32_t i, uint32_t j)
    {
        const std::pair<float, float>& p1 = sorting[i];
        const std::pair<float, float>& p2 = sorting[j];
        if (fabs(p1.first - p2.first) < 0.05f)
            // If same HP (max 5% difference) use shorter distance
            return p1.second < p2.second;
        return p1.first < p2.first;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}

}
}
