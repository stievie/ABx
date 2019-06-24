
#pragma once

#include "../AiTask.h"
#include "../Npc.h"

namespace AI {

AI_TASK(Die)
{
    (void)deltaMillis;
    Game::Npc& npc = chr.GetNpc();
    if (npc.Die())
        return ai::TreeNodeStatus::FINISHED;
    return ai::TreeNodeStatus::FAILED;
}

}
