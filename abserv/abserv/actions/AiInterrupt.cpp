#include "stdafx.h"
#include "AiInterrupt.h"
#include "../AiAgent.h"
#include "../Npc.h"
#include "../Game.h"
#include "Logger.h"

//#define DEBUG_AI

namespace AI {
namespace Actions {

Interrupt::Interrupt(const ArgumentsType& arguments) :
    Action(arguments)
{
    if (arguments.size() > 0)
        type_ = static_cast<AB::Entities::SkillType>(atoi(arguments[0].c_str()));
}

Node::Status Interrupt::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (IsCurrentAction(agent))
    {
        if (auto* cs = npc.GetCurrentSkill())
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

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectInterrupt,
        Game::SkillTargetTarget, type_);
    if (skillIndex == -1)
    {
#ifdef DEBUG_AI
        LOG_DEBUG << "No skill" << std::endl;
#endif
        return Status::Failed;
    }

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return Status::Failed;
    if (npc.IsDead() || !npc.resourceComp_->HaveEnoughResources(skill.get()))
        return Status::Failed;

    GetAgent(agent).selectedSkill_ = skillIndex;
    if (!npc.IsInRange(skill->GetRange(), target))
        return Status::Failed;

#ifdef DEBUG_AI
    LOG_DEBUG << "Interrupting " << target->GetName() << " Skill " << skill->data_.name << std::endl;
#endif

    if (!npc.SetSelectedObjectById(selection[0]))
        return Status::Failed;
    if (npc.UseSkill(skillIndex, false))
        return Status::Running;
    return Status::Failed;
}

}
}
