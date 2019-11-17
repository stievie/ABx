#include "stdafx.h"
#include "../Game.h"
#include "../Npc.h"
#include "../Skill.h"
#include "../AiAgent.h"
#include "AiIsInSkillRange.h"

namespace AI {
namespace Conditions {

bool IsInSkillRange::Evaluate(Agent& agent, const Node&)
{
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return true;

    // The skill must be selected by the action, so the actionn must be executed once
    // before this conditions returns something useful.
    // If the skill is executed but the target is out of range the action fails,
    // but stores the selected skill. So the second time it tries to execute the
    // skill this will return the real result.
    // That's why it never fails unless everything is setup correctly.
    int skillIndex = GetAgent(agent).selectedSkill_;
    if (skillIndex == -1)
        return true;

    const auto& npc = GetNpc(agent);
    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return true;
    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return true;

    return npc.IsInRange(skill->GetRange(), target.get());
}

}

}
