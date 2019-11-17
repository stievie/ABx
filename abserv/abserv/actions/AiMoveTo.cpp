#include "stdafx.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "AiMoveTo.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status MoveTo::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

#ifdef DEBUG_AI
    LOG_DEBUG << "Moving to " << selection[0] << std::endl;
#endif

    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return Status::Failed;
    if (IsCurrentAction(agent))
    {
        if (npc.IsInRange(Game::Ranges::Touch, target.get()))
            return Status::Finished;
    }

    npc.FollowObject(selection[0]);
    return Status::Running;
}

}
}
