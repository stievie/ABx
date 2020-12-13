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


#include "AiSelectAttackers.h"
#include "../Npc.h"
#include "../Group.h"
#include "../DamageComp.h"
#include <abscommon/Logger.h>

//#define DEBUG_AI

namespace AI {
namespace Filters {

SelectAttackers::SelectAttackers(const ArgumentsType& arguments) :
    Filter(arguments)
{
}

void SelectAttackers::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();

    std::map<uint32_t, float> sorting;
    Game::Npc& chr = GetNpc(agent);

    auto getAttackersOfActor = [&sorting, &entities](const Game::Actor& actor)
    {
        actor.VisitEnemiesInRange(Game::Ranges::Compass, [&](const Game::Actor& o)
        {
            if (o.IsDead())
                return Iteration::Continue;
            if (o.attackComp_->IsTarget(&actor) || actor.damageComp_->IsLastDamager(o))
            {
                entities.push_back(o.id_);
                sorting[o.id_] = o.GetDistance(&actor);
            }
            return Iteration::Continue;
        });
    };

    if (auto group = chr.GetGroup())
    {
#ifdef DEBUG_AI
        LOG_DEBUG << "Selecting group attackers, group members " << group->GetMemberCount() << std::endl;
#endif
        group->VisitMembers([&](const Game::Actor& current) -> Iteration
        {
            getAttackersOfActor(current);
            return Iteration::Continue;
        });
    }
    else
    {
        getAttackersOfActor(chr);
    }

#ifdef DEBUG_AI
    LOG_DEBUG << "Selected " << entities.size() << std::endl;
#endif
    std::sort(entities.begin(), entities.end(), [&sorting](uint32_t i, uint32_t j)
    {
        const float& p1 = sorting[i];
        const float& p2 = sorting[j];
        return p1 < p2;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}

}
}
