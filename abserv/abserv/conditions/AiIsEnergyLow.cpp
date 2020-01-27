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
#include "AiIsEnergyLow.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsEnergyLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    const auto& skillBar = *npc.skills_;
    int skillCount = 0;
    int sumEnergy = 0;
    // Get avg. skill cost
    skillBar.VisitSkills([&](int, const Game::Skill& current)
    {
        ++skillCount;
        sumEnergy += current.energy_;
        return Iteration::Continue;
    });

    if (skillCount > 0)
    {
        int avgEnergy = sumEnergy / skillCount;
        return npc.resourceComp_->GetEnergy() <= avgEnergy;
    }
    return npc.resourceComp_->GetEnergyRatio() <= LOW_ENERGY_THRESHOLD;
}

}
}
