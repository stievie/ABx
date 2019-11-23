#include "stdafx.h"
#include "AiUseDamageSkill.h"
#include "../Npc.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status UseDamageSkill::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (IsCurrentAction(agent))
    {
        if (auto cs = npc.GetCurrentSkill())
        {
            if (cs->IsUsing())
                return Status::Running;
        }
        return Status::Finished;
    }

    if (npc.skills_->GetCurrentSkill())
        // Some other skill currently using
        return Status::Failed;

    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return Status::Failed;
    if (!target->IsActorType())
        return Status::Failed;

    const Game::Actor& actor = Game::To<Game::Actor>(*target);
    if (actor.IsDead())
        return Status::Failed;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectDamage, targetType_);
    if (skillIndex == -1)
        return Status::Failed;

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return Status::Failed;
    if (npc.IsDead() || !npc.resourceComp_->HaveEnoughResources(skill.get()))
        return Status::Failed;

    GetAgent(agent).selectedSkill_ = skillIndex;
    if (!npc.IsInRange(skill->GetRange(), target.get()))
        return Status::Failed;

    npc.SetSelectedObjectById(selection[0]);
    npc.UseSkill(skillIndex);

    return Status::Running;
}

}
}
