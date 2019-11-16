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

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectGainEnergy, Game::SkillTargetSelf);
    if (skillIndex == -1)
        return Status::Failed;

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return Status::Failed;
    if (npc.IsDead() || !npc.resourceComp_->HaveEnoughResources(skill.get()))
        return Status::Failed;

    npc.UseSkill(skillIndex);
    return Status::Running;
}

}
}
