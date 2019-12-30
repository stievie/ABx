#include "stdafx.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "AiHealSelf.h"
#include "../Game.h"

namespace AI {
namespace Actions {

HealSelf::HealSelf(const ArgumentsType& arguments) :
    SkillAction(arguments)
{ }

Node::Status HealSelf::DoAction(Agent& agent, uint32_t)
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
    if (npc.IsDead())
        return Status::Failed;

    if (npc.skills_->GetCurrentSkill())
        // Some other skill currently using
        return Status::Failed;

    std::vector<int> skills;
    npc.GetSkillCandidates(skills, Game::SkillEffectHeal, Game::SkillTargetSelf);

    int skillIndex = GetSkillIndex(skills, npc, &npc);
    if (skillIndex == -1)
    {
        const auto& selection = agent.filteredAgents_;
        if (selection.size() == 0)
            return Status::Failed;

        auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
        if (target)
        {
            if (npc.GetSkillCandidates(skills, Game::SkillEffectHeal, Game::SkillTargetSelf,
                                       AB::Entities::SkillTypeAll, target))
                skillIndex = GetSkillIndex(skills, npc, target);
        }
        if (skillIndex == -1)
            return Status::Failed;
        if (target)
        {
            if (!npc.SelectedObjectById(target->GetId()))
                return Status::Failed;
        }
    }
    else
    {
        if (!npc.SelectedObjectById(npc.GetId()))
            return Status::Failed;
    }


    if (npc.UseSkill(skillIndex, false))
        return Status::Running;

#ifdef DEBUG_AI
    auto skill = npc.skills_->GetSkill(skillIndex);
    LOG_DEBUG << npc.GetName() << " failed to use skill " << (skill ? skill->data_.name : "(null)") << std::endl;
#endif
    return Status::Failed;
}

}
}
