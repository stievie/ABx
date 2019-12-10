#include "stdafx.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "AiHealOther.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status HealOther::DoAction(Agent& agent, uint32_t)
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

    // Possible heal targets
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    // Lambda to use the given skills on the target
    const auto useSkill = [&npc](int skillIndex, uint32_t targetId) -> bool
    {
        auto skill = npc.skills_->GetSkill(skillIndex);
        if (!skill)
            return false;
        if (!npc.resourceComp_->HaveEnoughResources(skill.get()))
            return false;
        if (!npc.SelectedObjectById(targetId))
            return false;
        return npc.UseSkill(skillIndex, false);
    };

    if (selection.size() > 4)
    {
        // Many to heal try party healing
        int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectHeal, Game::SkillTargetParty);
        if (skillIndex != -1)
        {
            GetAgent(agent).selectedSkill_ = skillIndex;
            if (useSkill(skillIndex, selection[0]))
                return Status::Running;
        }
    }

    // Highest priorioty target is on top
    auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
    if (!target)
        return Status::Failed;
    if (target->IsDead())
        return Status::Failed;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectHeal, Game::SkillTargetTarget);
    if (skillIndex == -1)
        return Status::Failed;
    GetAgent(agent).selectedSkill_ = skillIndex;

    if (useSkill(skillIndex, selection[0]))
        return Status::Running;

#ifdef DEBUG_AI
    LOG_DEBUG << npc.GetName() << " failed to use skill " << skill->data_.name << std::endl;
#endif
    return Status::Failed;
}

HealOther::HealOther(const ArgumentsType& arguments) :
    Action(arguments)
{
    mustComplete_ = true;
}

}
}
