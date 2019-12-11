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

    if (npc.skills_->GetCurrentSkill())
        // Some other skill currently using
        return Status::Failed;

    std::vector<int> skills;
    if (!npc.GetSkillCandidates(skills, Game::SkillEffectHeal, Game::SkillTargetSelf))
    {
#ifdef DEBUG_AI
    LOG_DEBUG << "No skill found" << std::endl;
#endif
        return Status::Failed;
    }

    int skillIndex = GetSkillIndex(skills, npc, &npc);
    if (skillIndex == -1)
    {
        const auto& selection = agent.filteredAgents_;
        auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
        if (target)
            skillIndex = GetSkillIndex(skills, npc, target);
        if (skillIndex == -1)
            return Status::Failed;
    }

    if (npc.IsDead())
        return Status::Failed;

    if (!npc.SelectedObjectById(npc.GetId()))
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
