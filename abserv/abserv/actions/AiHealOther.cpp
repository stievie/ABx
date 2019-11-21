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
        npc.SetSelectedObjectById(targetId);
        npc.UseSkill(skillIndex);
        return true;
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
    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return Status::Failed;
    if (!target->IsActorType())
        return Status::Failed;

    const Game::Actor& actor = Game::To<Game::Actor>(*target);
    if (actor.IsDead())
        return Status::Failed;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectHeal, Game::SkillTargetTarget);
    if (skillIndex == -1)
        return Status::Failed;
    GetAgent(agent).selectedSkill_ = skillIndex;

    if (useSkill(skillIndex, selection[0]))
    {
        return Status::Running;
    }
    return Status::Failed;
}

HealOther::HealOther(const ArgumentsType& arguments) :
    Action(arguments)
{
    mustComplete_ = true;
}

}
}
