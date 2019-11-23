#include "stdafx.h"
#include "AiSelectMob.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {
namespace Filters {

void SelectMob::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    std::map<uint32_t, size_t> sorting;
    Game::Npc& chr = GetNpc(agent);
    chr.VisitEnemiesInRange(Game::Ranges::HalfCompass, [&](const Game::Actor& o)
    {

        if (o.IsSelectable() && !o.IsUndestroyable())
        {
            size_t c = o.GetAllyCountInRange(Game::Ranges::Casting);
            entities.push_back(o.id_);
            sorting[o.id_] = c;
        }
        return Iteration::Continue;
    });
    // Sort by sorrounding allys
    std::sort(entities.begin(), entities.end(), [&sorting](uint32_t i, uint32_t j)
    {
        const size_t& p1 = sorting[i];
        const size_t& p2 = sorting[j];
        return p1 < p2;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}

}
}
