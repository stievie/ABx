#pragma once

#include "../AiTask.h"
#include "../Npc.h"

namespace AI {

AI_TASK(AttackSelection)
{
    Game::Npc& npc = chr.GetNpc();
    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
    {
        return ai::TreeNodeStatus::FAILED;
    }
    for (ai::CharacterId id : selection)
    {
        if (npc.AttackById(id))
            return ai::TreeNodeStatus::FINISHED;
    }
    return ai::TreeNodeStatus::FAILED;
}

}