#include "stdafx.h"
#include "AiMoveIntoSkillRange.h"
#include "../AiAgent.h"
#include "../Npc.h"
#include "../Skill.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status MoveIntoSkillRange::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto* target = npc.GetGame()->GetObject<Game::GameObject>(selection[0]);
    if (!target)
        return Status::Failed;
    if (!Game::Is<Game::Actor>(target))
        return Status::Failed;

    Game::Actor& actor = Game::To<Game::Actor>(*target);

    const AiAgent& a = GetAgent(agent);
    int skillIndex = a.selectedSkill_;
    if (skillIndex == -1)
        return Status::Failed;

    Game::Skill* skill = npc.GetSkill(static_cast<uint32_t>(skillIndex));
    if (!skill)
        return Status::Failed;

    if (IsCurrentAction(agent))
    {
        if (skill->IsInRange(&actor))
            return Status::Finished;
    }

    if (npc.FollowObjectById(actor.GetId(), false))
        return Status::Running;
    return Status::Failed;
}

}
}
