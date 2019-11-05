#include "stdafx.h"
#include "AiSelectAggro.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

SelectAggro::SelectAggro(const ArgumentsType& arguments) :
    Filter(arguments)
{ }

void SelectAggro::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    Game::Npc& chr = GetNpc(agent);
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor* o)
    {
        entities.push_back(o->id_);
    });
}

}
}
