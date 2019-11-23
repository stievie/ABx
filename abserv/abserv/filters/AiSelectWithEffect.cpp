#include "stdafx.h"
#include "AiSelectWithEffect.h"
#include "../Npc.h"
#include "../EffectManager.h"

namespace AI {
namespace Filters {

SelectWithEffect::SelectWithEffect(const ArgumentsType& arguments) :
    Filter(arguments)
{
    if (arguments.size() > 0)
        effectCat_ = Game::EffectCatNameToEffectCat(arguments[0]);

    if (arguments.size() > 1)
    {
        const std::string& value = arguments.at(1);
        if (value.compare("friend") == 0)
            class_ = Game::TargetClass::Friend;
        else if (value.compare("foe") == 0)
            class_ = Game::TargetClass::Foe;
    }
}

void SelectWithEffect::Execute(Agent &agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);

    chr.VisitInRange(Game::Ranges::Aggro, [&](const Game::GameObject& current)
    {
        if (!Game::Is<Game::Actor>(current))
            return Iteration::Continue;

        const Game::Actor& actor = Game::To<Game::Actor>(current);
        if (Game::TargetClassMatches(chr, class_, actor) &&
            actor.effectsComp_->HasEffectOf(effectCat_))
            entities.push_back(actor.id_);

        return Iteration::Continue;
    });
}

}
}
