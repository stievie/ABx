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


#include "AiSelectAttackTarget.h"
#include "../Npc.h"
#include <abshared/Mechanic.h>
#include "../Game.h"
#include "../Group.h"

namespace AI {
namespace Filters {

void SelectAttackTarget::Execute(Agent& agent)
{
    AgentIds candidates;
    Game::Npc& chr = GetNpc(agent);
    std::map<uint32_t, float> sorting;
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor& o)
    {
        if (!o.IsSelectable() || o.IsDead() || o.IsUndestroyable())
            return Iteration::Continue;

        candidates.push_back(o.id_);
        // Bigger is more likely to be selected
        sorting[o.id_] = chr.GetAggro(&o);
        return Iteration::Continue;
    });

    auto& entities = agent.filteredAgents_;
    if (entities.size() == 1)
    {
        // Only one already selected
        uint32_t selected = entities.front();
        const auto it = std::find_if(candidates.begin(), candidates.end(), [&selected] (uint32_t current)
        {
            return selected == current;
        });
        // If the old selected is also in the candidates list, use this, regardless of the score.
        // This prevents switching targets.
        if (it != candidates.end())
        {
            auto* target = chr.GetGame()->GetObject<Game::Actor>(*it);
            if (target && !target->IsDead())
                // Not dead yet.
                return;
        }
    }

    entities.clear();
    if (candidates.size() == 0)
    {
        Game::Group* crowd = chr.GetGroup();
        if (!crowd)
            return;
        crowd->VisitMembers([&](const Game::Actor& current)
        {
            auto* target = current.attackComp_->GetCurrentTarget();
            if (target)
            {
                if (!target->IsSelectable() || target->IsDead() || target->IsUndestroyable())
                    return Iteration::Continue;

                candidates.push_back(target->id_);
                // Bigger is more likely to be selected
                sorting[target->id_] = chr.GetAggro(target);
                return Iteration::Continue;
            }
            return Iteration::Continue;
        });
    }
    if (candidates.size() == 0)
        return;

    std::sort(candidates.begin(), candidates.end(), [&sorting](uint32_t i, uint32_t j)
    {
        return sorting[i] > sorting[j];
    });

    entities.push_back(candidates.front());
}

}
}
