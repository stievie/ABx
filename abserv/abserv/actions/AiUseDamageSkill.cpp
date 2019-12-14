#include "stdafx.h"
#include "AiUseDamageSkill.h"
#include "../Npc.h"
#include "../Game.h"

//#define DEBUG_AI

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

    auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
    if (!target)
        return Status::Failed;
    if (target->IsDead())
        return Status::Failed;

    std::vector<int> skills;
    if (!npc.GetSkillCandidates(skills, Game::SkillEffectDamage, targetType_,
                                AB::Entities::SkillTypeAll, target))
    {
#ifdef DEBUG_AI
//    LOG_DEBUG << npc.GetName() << " no skill found" << std::endl;
#endif
        return Status::Failed;
    }

    int skillIndex = GetSkillIndex(skills, npc, target);
    if (skillIndex == -1)
    {
#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " no skill found" << std::endl;
#endif
        return Status::Failed;
    }

    auto skill = npc.skills_->GetSkill(skillIndex);
    GetAgent(agent).selectedSkill_ = skillIndex;
    if (!npc.IsInRange(skill->GetRange(), target))
        return Status::Failed;

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
