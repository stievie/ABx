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


#include "AiResurrectSelection.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include "../Game.h"

#undef DEBUG_AI

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

    ea::vector<int> skills;
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
