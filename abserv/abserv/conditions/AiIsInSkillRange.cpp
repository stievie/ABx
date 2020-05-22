/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


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

    // The skill must be selected by the action, so the action must be executed once
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
    auto* target = npc.GetGame()->GetObject<Game::GameObject>(selection[0]);
    if (!target)
        return true;

    return npc.IsInRange(skill->GetRange(), target);
}

}
}
