#include "stdafx.h"
#include "AiResurrectSelection.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status ResurrectSelection::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (agent.context_.IsActionRunning(id_))
    {
        if (auto cs = npc.GetCurrentSkill())
        {
            if (cs->IsUsing())
                return Status::Running;
        }
        return Status::Finished;
    }

    auto currSkill = npc.skills_->GetCurrentSkill();
    if (currSkill)
        // Some other skill currently using
        return Status::Failed;

    // Possible heal targets
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return Status::Failed;
    if (!target->IsActorType())
        return Status::Failed;

    Game::Actor* actor = Game::To<Game::Actor>(target.get());
    if (!actor->IsDead())
        return Status::Failed;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectResurrect, Game::SkillTargetTarget, actor);
    if (skillIndex == -1)
        return Status::Failed;

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return Status::Failed;
    if (!npc.resourceComp_->HaveEnoughResources(skill.get()))
        return Status::Failed;

    npc.SetSelectedObjectById(selection[0]);
    npc.UseSkill(skillIndex);
    return Status::Running;
}

}
}
