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
        ea::vector<int> skills;
        if (npc.GetSkillCandidates(skills, Game::SkillEffectHeal, Game::SkillTargetParty))
        {
            int skillIndex = GetSkillIndex(skills, npc, nullptr);
            if (skillIndex != -1)
            {
                GetAgent(agent).selectedSkill_ = skillIndex;
                if (useSkill(skillIndex, selection[0]))
                    return Status::Running;
            }
        }
    }

    // Highest priorioty target is on top
    auto* target = npc.GetGame()->GetObject<Game::Actor>(selection[0]);
    if (!target)
        return Status::Failed;
    if (target->IsDead())
        return Status::Failed;

    ea::vector<int> skills;
    if (!npc.GetSkillCandidates(skills, Game::SkillEffectHeal, Game::SkillTargetTarget))
        return Status::Failed;

    int skillIndex = GetSkillIndex(skills, npc, target);
    if (skillIndex == -1)
        return Status::Failed;
    GetAgent(agent).selectedSkill_ = skillIndex;

    if (useSkill(skillIndex, selection[0]))
        return Status::Running;

#ifdef DEBUG_AI
    auto skill = npc.skills_->GetSkill(skillIndex);
    LOG_DEBUG << npc.GetName() << " failed to use skill " << (skill ? skill->data_.name : "(null)") << std::endl;
#endif
    return Status::Failed;
}

HealOther::HealOther(const ArgumentsType& arguments) :
    SkillAction(arguments)
{ }

}
}
