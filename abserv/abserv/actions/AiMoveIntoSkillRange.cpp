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
#include "AiMoveIntoSkillRange.h"
#include "../AiAgent.h"
#include "../Npc.h"
#include "../Skill.h"
#include "../Game.h"

namespace AI {
namespace Actions {

Node::Status MoveIntoSkillRange::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return Status::Failed;

    auto* target = npc.GetGame()->GetObject<Game::GameObject>(selection[0]);
    if (!target)
        return Status::Failed;
    if (!Game::Is<Game::Actor>(target))
        return Status::Failed;

    Game::Actor& actor = Game::To<Game::Actor>(*target);

    const AiAgent& a = GetAgent(agent);
    int skillIndex = a.selectedSkill_;
    if (skillIndex == -1)
        return Status::Failed;

    Game::Skill* skill = npc.GetSkill(static_cast<uint32_t>(skillIndex));
    if (!skill)
        return Status::Failed;

    if (IsCurrentAction(agent))
    {
        if (skill->IsInRange(&actor))
            return Status::Finished;
    }

    if (npc.FollowObjectById(actor.GetId(), false))
        return Status::Running;
    return Status::Failed;
}

}
}
