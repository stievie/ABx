#include "stdafx.h"
#include "AiResurrectSelection.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "../Game.h"

//#define DEBUG_AI

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

    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
    if (!target)
        return Status::Failed;
    if (!target->IsDead())
        return Status::Failed;

    std::vector<int> skills;
    if (!npc.GetSkillCandidates(skills, Game::SkillEffectResurrect, Game::SkillTargetTarget))
    {
#ifdef DEBUG_AI
    LOG_DEBUG << "No skill found" << std::endl;
#endif
        return Status::Failed;
    }

    int skillIndex = GetSkillIndex(skills, npc, target);
    if (skillIndex == -1)
        return Status::Failed;

    auto skill = npc.skills_->GetSkill(skillIndex);
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
    if (!npc.SelectedObjectById(selection[0]))
        return Status::Failed;
    if (npc.UseSkill(skillIndex, false))
        return Status::Running;

#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " failed to use skill " << skill->data_.name << std::endl;
#endif
    return Status::Failed;
}

}
}
