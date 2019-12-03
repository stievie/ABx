#include "stdafx.h"
#include "AiSelectAttackTarget.h"
#include "../Npc.h"
#include "../Mechanic.h"
#include "../Game.h"
#include "../Group.h"

namespace AI {
namespace Filters {

void SelectAttackTarget::Execute(Agent& agent)
{
    AgentIds candidates;
    Game::Npc& chr = GetNpc(agent);
    std::map<uint32_t, float> sorting;
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor& o)
    {
        if (!o.IsSelectable() || o.IsDead() || o.IsUndestroyable())
            return Iteration::Continue;

        candidates.push_back(o.id_);
        // Bigger is more likely to be selected
        sorting[o.id_] = chr.GetAggro(&o);
        return Iteration::Continue;
    });

    auto& entities = agent.filteredAgents_;
    if (entities.size() == 1)
    {
        // Only one already selected
        uint32_t selected = entities.front();
        const auto it = std::find_if(candidates.begin(), candidates.end(), [&selected] (uint32_t current)
        {
            return selected == current;
        });
        // If the old selected is also in the candidates list, use this, regardless of the score.
        // This prevents switching targets.
        if (it != candidates.end())
        {
            auto* t = chr.GetGame()->GetObject<Game::Actor>(*it);
            if (!t->IsDead())
                // Not dead yet.
                return;
        }
    }

    entities.clear();
    if (candidates.size() == 0)
    {
        Game::Group* crowd = chr.GetGroup();
        if (!crowd)
            return;
        crowd->VisitMembers([&](const Game::Actor& current)
        {
            auto* target = current.attackComp_->GetCurrentTarget();
            if (target)
            {
                if (!target->IsSelectable() || target->IsDead() || target->IsUndestroyable())
                    return Iteration::Continue;

                candidates.push_back(target->id_);
                // Bigger is more likely to be selected
                sorting[target->id_] = chr.GetAggro(target);
                return Iteration::Continue;
            }
            return Iteration::Continue;
        });
    }
    if (candidates.size() == 0)
        return;

    std::sort(candidates.begin(), candidates.end(), [&sorting](uint32_t i, uint32_t j)
    {
        return sorting[i] > sorting[j];
    });

    entities.push_back(candidates.front());
}

}
}
