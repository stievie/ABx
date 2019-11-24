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

    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return Status::Failed;
    if (!target->IsActorType())
        return Status::Failed;

    const Game::Actor& actor = Game::To<Game::Actor>(*target);
    if (npc.autorunComp_->IsFollowing(actor))
        return Status::Running;

    npc.FollowObject(target);
    return Status::Running;
}

}

}
