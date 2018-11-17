#pragma once

#include "../AiTask.h"
#include "../Npc.h"

namespace AI {

AI_TASK(AttackOnSelection)
{
    Game::Npc& npc = chr.GetNpc();
    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
    {
        return ai::TreeNodeStatus::FAILED;
    }
    bool attacked = false;
    for (ai::CharacterId id : selection)
    {
        //attacked |= npc.attack(id);
    }
    return attacked ? ai::TreeNodeStatus::FINISHED : ai::TreeNodeStatus::FAILED;
}

}