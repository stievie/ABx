#include "stdafx.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "AiGainEnergy.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status GainEnergy::DoAction(Agent& agent, uint32_t)
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

    auto currSkill = npc.skills_->GetCurrentSkill();
    if (currSkill)
        // Some other skill currently using
        return Status::Failed;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectGainEnergy, Game::SkillTargetNone);
    if (skillIndex == -1)
        return Status::Failed;

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return Status::Failed;
    if (npc.IsDead() || !npc.resourceComp_->HaveEnoughResources(skill.get()))
        return Status::Failed;

    if (skill->HasTarget(Game::SkillTargetTarget))
    {
        const auto& selection = agent.filteredAgents_;
        if (selection.empty())
            return Status::Failed;
        auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
        if (!target)
            return Status::Failed;
        if (target->IsDead())
            return Status::Failed;

        GetAgent(agent).selectedSkill_ = skillIndex;
        if (!npc.IsInRange(skill->GetRange(), target))
            return Status::Failed;
        if (!npc.SelectedObjectById(selection[0]))
            return Status::Failed;
    }

    if (npc.UseSkill(skillIndex, false))
        return Status::Running;

#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " failed to use skill " << skill->data_.name << std::endl;
#endif
    return Status::Failed;
}

}
}
