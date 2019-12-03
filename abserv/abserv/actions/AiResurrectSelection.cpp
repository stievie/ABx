#include "stdafx.h"
#include "AiResurrectSelection.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "../Game.h"

#undef DEBUG_AI

namespace AI {
namespace Actions {

Node::Status ResurrectSelection::DoAction(Agent& agent, uint32_t)
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

    // Possible heal targets
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
    if (!target)
        return Status::Failed;
    if (!target->IsDead())
        return Status::Failed;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectResurrect, Game::SkillTargetTarget);
    if (skillIndex == -1)
    {
#ifdef DEBUG_AI
    LOG_DEBUG << "No skill found" << std::endl;
#endif
        return Status::Failed;
    }

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
    {
#ifdef DEBUG_AI
    LOG_DEBUG << "Skill " << skillIndex << " not found" << std::endl;
#endif
        return Status::Failed;
    }
    if (!npc.resourceComp_->HaveEnoughResources(skill.get()))
    {
#ifdef DEBUG_AI
    LOG_DEBUG << "Not enough resources to use skill" << std::endl;
#endif
        return Status::Failed;
    }
    GetAgent(agent).selectedSkill_ = skillIndex;
    if (!npc.IsInRange(skill->GetRange(), target))
    {
#ifdef DEBUG_AI
    LOG_DEBUG << "Target not in skill range" << std::endl;
#endif
        return Status::Failed;
    }

#ifdef DEBUG_AI
    LOG_DEBUG << "Resurrecting " << target->GetName() << std::endl;
#endif
    npc.SetSelectedObjectById(selection[0]);
    npc.UseSkill(skillIndex);
    return Status::Running;
}

}
}
