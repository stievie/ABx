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
#include "AiIsCloseToSelection.h"
#include "../Game.h"
#include "../Npc.h"
#include "../GameObject.h"

namespace AI {
namespace Conditions {

IsCloseToSelection::IsCloseToSelection(const ArgumentsType& arguments) :
    Condition(arguments)
{
    if (arguments.size() == 1)
        distance_ = static_cast<float>(atof(arguments[0].c_str()));
}

bool IsCloseToSelection::Evaluate(Agent& agent, const Node&)
{
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return false;

    auto& npc = AI::GetNpc(agent);
    auto game = npc.GetGame();

    const auto& ownPos = npc.GetPosition();
    for (auto id : selection)
    {
        auto* sel = game->GetObject<Game::GameObject>(id);
        const Math::Vector3& _pos = sel->GetPosition();
        const float distance = ownPos.Distance(_pos);
        if (distance > distance_)
            return false;
    }
    return true;
}

}
}
