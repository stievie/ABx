#pragma once

#include "../AiTask.h"
#include "../Npc.h"

namespace AI {

AI_TASK(MoveTo)
{
    (void)deltaMillis;
    Game::Npc& npc = chr.GetNpc();
    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
    {
        return ai::TreeNodeStatus::FAILED;
    }
#ifdef DEBUG_AI
    LOG_DEBUG << "Moving to " << selection[0] << std::endl;
#endif
    npc.FollowObject(selection[0]);
    return ai::TreeNodeStatus::FINISHED;
}

}
