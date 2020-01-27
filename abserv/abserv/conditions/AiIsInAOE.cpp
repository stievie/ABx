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
#include "AiIsInAOE.h"
#include "../Game.h"
#include "../Npc.h"
#include "../AreaOfEffect.h"

namespace AI {
namespace Conditions {

bool IsInAOE::Evaluate(Agent& agent, const Node&)
{
    auto& npc = GetNpc(agent);
    bool result = false;
    npc.VisitInRange(Game::Ranges::Aggro, [&result, &npc](const Game::GameObject& object)
    {
        if (Game::Is<Game::AreaOfEffect>(object))
        {
            const auto& aoe = Game::To<Game::AreaOfEffect>(object);
            if (aoe.IsEnemy(&npc) && aoe.HasEffect(Game::SkillEffectDamage) && aoe.IsInRange(&npc))
            {
                result = true;
                return Iteration::Break;
            }
        }
        return Iteration::Continue;
    });
    return result;
}

}
}
