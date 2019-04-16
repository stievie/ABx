#pragma once

#include "../AiTask.h"
#include "../Npc.h"
#include "../Game.h"

namespace AI {

AI_TASK(AttackSelection)
{
    Game::Npc& npc = chr.GetNpc();
    if (chr.currentTask_ == this)
    {
        if (auto t = chr.currentTarget_.lock())
        {
            // Attack until dead
            if (!t->IsDead())
                return ai::RUNNING;
            chr.currentTask_ = nullptr;
            chr.currentTarget_.reset();
            return ai::FINISHED;
        }
        chr.currentTarget_.reset();
        chr.currentTask_ = nullptr;
        return ai::FINISHED;
    }

    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
    {
        return ai::TreeNodeStatus::FAILED;
    }
    for (auto id : selection)
    {
        auto target = npc.GetGame()->GetObjectById(id);
        if (!target)
            continue;
        Game::Actor* actor = dynamic_cast<Game::Actor*>(target.get());
        if (!actor || actor->IsDead())
            continue;

        if (npc.AttackById(id))
        {
            chr.currentTask_ = this;
            chr.currentTarget_ = std::static_pointer_cast<Game::Actor>(target);
            return ai::TreeNodeStatus::RUNNING;
        }
    }
    return ai::TreeNodeStatus::FAILED;
}

}