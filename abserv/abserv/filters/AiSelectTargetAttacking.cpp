#include "stdafx.h"
#include "AiSelectTargetAttacking.h"
#include "../AiAgent.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

void SelectTargetAttacking::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    chr.VisitEnemiesInRange(Game::Ranges::Casting, [&](const Game::Actor& current)
    {
        if (current.attackComp_->IsHitting())
        {
            entities.push_back(current.id_);
            return Iteration::Break;
        }
        if (auto* skill = current.GetCurrentSkill())
        {
            if (skill->IsType(AB::Entities::SkillTypeAttack))
            {
                entities.push_back(current.id_);
                return Iteration::Break;
            }
        }
        return Iteration::Continue;
    });
}

}
}
