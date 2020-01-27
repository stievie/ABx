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
#include "SkillAction.h"
#include "../Actor.h"
#include "Agent.h"
#include "../Game.h"
#include "../AiAgent.h"
#include "../Npc.h"
#include <AB/ProtocolCodes.h>

//#define DEBUG_AI

namespace AI {
namespace Actions {

bool SkillAction::TestSkill(int index, Game::Actor& source, Game::Actor* target)
{
    auto skill = source.skills_->GetSkill(index);
    if (!skill)
        return false;
    if (!source.resourceComp_->HaveEnoughResources(skill.get()))
        return false;
    if (skill->NeedsTarget())
    {
        if (!target)
            return false;
        if (target->IsDead())
        {
            if (!skill->HasEffect(Game::SkillEffectResurrect))
                return false;
            if (!source.IsAlly(target))
                return false;
        }
    }
    auto res = skill->CanUse(&source, target);
#ifdef DEBUG_AI
    if (res != AB::GameProtocol::SkillErrorNone)
        LOG_DEBUG << skill->data_.name << "::CanUse() returned " << static_cast<int>(res) << std::endl;
#endif
    // Out of range is okay we will move to the target
    return res == AB::GameProtocol::SkillErrorNone || res == AB::GameProtocol::SkillErrorOutOfRange;
}

int SkillAction::GetSkillIndex(std::vector<int>& candidates, Game::Actor& source, Game::Actor* target)
{
    int skillIndex = -1;
    for (int index : candidates)
    {
        if (TestSkill(index, source, target))
        {
            skillIndex = index;
            break;
        }
    }
    return skillIndex;
}

}

}
