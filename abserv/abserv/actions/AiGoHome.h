#pragma once

#include "../AiTask.h"
#include "../Npc.h"

namespace AI {

AI_TASK(GoHome)
{
    (void)deltaMillis;
    Game::Npc& npc = chr.GetNpc();
    if (npc.GotoHomePos())
        return ai::TreeNodeStatus::FINISHED;
    return ai::TreeNodeStatus::FAILED;
}

}
