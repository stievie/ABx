#include "stdafx.h"
#include "AiSelectVisible.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

SelectVisible::SelectVisible(const ArgumentsType& arguments) :
    Filter(arguments)
{
}

void SelectVisible::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    chr.VisitInRange(Game::Ranges::Compass, [&](const Game::GameObject& o)
    {
        entities.push_back(o.id_);
        return Iteration::Continue;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}

}
}
