#include "stdafx.h"
#include "AiSelectTargetUsingSkill.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

SelectTargetUsingSkill::SelectTargetUsingSkill(const ArgumentsType& arguments) :
    Filter(arguments)
{
    if (arguments.size() > 0)
        type_ = static_cast<AB::Entities::SkillType>(atoi(arguments[0].c_str()));

    if (arguments.size() > 1)
    {
        const std::string& value = arguments.at(1);
        if (value.compare("friend") == 0)
            class_ = Game::TargetClass::Friend;
        else if (value.compare("foe") == 0)
            class_ = Game::TargetClass::Foe;
    }

    if (arguments.size() > 2)
    {
        const std::string& value = arguments.at(2);
        minActivationTime_ = atoi(value.c_str());
    }
}

void SelectTargetUsingSkill::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    Id selection = INVALID_ID;
    chr.VisitInRange(Game::Ranges::Casting, [&](const Game::GameObject& current)
    {
        if (!Game::Is<Game::Actor>(current))
            return Iteration::Continue;

        const Game::Actor& actor = Game::To<Game::Actor>(current);
        if (Game::TargetClassMatches(class_, actor) &&
            actor.IsSelectable() && !actor.IsUndestroyable() && !actor.IsDead())
        {
            const auto* skill = actor.skills_->GetCurrentSkill();
            if (skill && skill->IsUsing() && skill->IsType(type_) &&
                (skill->activation_ > minActivationTime_))
            {
                selection = actor.id_;
                return Iteration::Break;
            }
        }
        return Iteration::Continue;
    });
    if (selection != INVALID_ID)
        entities.push_back(selection);
}

}
}
