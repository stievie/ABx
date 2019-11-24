#include "stdafx.h"
#include "AiSelectVisible.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

SelectVisible::SelectVisible(const ArgumentsType& arguments) :
    Filter(arguments)
{
    if (arguments.size() > 0)
    {
        const std::string& value = arguments.at(0);
        if (value.compare("friend") == 0)
            class_ = Game::TargetClass::Friend;
        else if (value.compare("foe") == 0)
            class_ = Game::TargetClass::Foe;
    }

    if (arguments.size() > 1)
        range_ = static_cast<Game::Ranges>(atoi(arguments[1].c_str()));
}

void SelectVisible::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    chr.VisitInRange(range_, [&](const Game::GameObject& o)
    {
        if (!Game::Is<Game::Actor>(o))
            return Iteration::Continue;

        const Game::Actor& actor = Game::To<Game::Actor>(o);

        if (!Game::TargetClassMatches(chr, class_, actor))
            return Iteration::Continue;

        if (chr.IsObjectInSight(o))
            entities.push_back(o.id_);
        return Iteration::Continue;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}

}
}
