#include "stdafx.h"
#include "AiSelectAttackers.h"
#include "../Npc.h"

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
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        if (o->attackComp_->IsTarget(&chr))
        {
            entities.push_back(o->id_);
            sorting[o->id_] = o->GetDistance(&chr);
        }
    });
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
