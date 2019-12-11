#include "stdafx.h"
#include "AiSelectTargetUsingSkill.h"
#include "../Npc.h"
#include "Logger.h"
#include "Subsystems.h"
#include "Random.h"

//#define DEBUG_AI

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
    auto& rng = *GetSubsystem<Crypto::Random>();
    chr.VisitInRange(Game::Ranges::Casting, [&](const Game::GameObject& current)
    {
        if (!Game::Is<Game::Actor>(current))
            return Iteration::Continue;

        const Game::Actor& actor = Game::To<Game::Actor>(current);
        if (!Game::TargetClassMatches(chr, class_, actor))
            return Iteration::Continue;
        if (!actor.IsSelectable() || actor.IsDead() || actor.IsUndestroyable())
            return Iteration::Continue;

        if (actor.IsUsingSkillOfType(type_, minActivationTime_))
        {
            if (rng.GetFloat() < 0.3f)
                return Iteration::Continue;
#ifdef DEBUG_AI
            LOG_DEBUG << "Selected " << actor.GetName() << std::endl;
#endif
            entities.push_back(actor.id_);
            return Iteration::Break;
        }
        return Iteration::Continue;
    });
}

}
}
