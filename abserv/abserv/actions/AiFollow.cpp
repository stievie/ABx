#include "stdafx.h"
#include "AiFollow.h"
#include "../Npc.h"
#include "../Game.h"
#include "../AutoRunComp.h"

namespace AI {
namespace Actions {

Node::Status Follow::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);

    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
    if (!target)
        return Status::Failed;

    if (npc.GetPosition().Equals(target->GetPosition(), Game::AT_POSITION_THRESHOLD))
        return Status::Finished;

    if (npc.autorunComp_->IsFollowing(*target))
        return Status::Running;

    if (npc.FollowObject(target, false))
        return Status::Running;
    return Status::Failed;
}

}

}
