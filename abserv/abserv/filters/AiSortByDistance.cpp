#include "stdafx.h"
#include "AiSortByDistance.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

void SortByDistance::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    if (entities.size() == 0)
        return;

    Game::Npc& chr = GetNpc(agent);
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
