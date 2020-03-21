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
#include "AiSelectTargetUsingSkill.h"
#include "../Npc.h"
#include <abscommon/Random.h>

//#define DEBUG_AI

namespace AI {
namespace Filters {

SelectTargetUsingSkill::SelectTargetUsingSkill(const ArgumentsType& arguments) :
    Filter(arguments)
{
    GetArgument<AB::Entities::SkillType>(arguments, 0, type_);
    std::string ff;
    if (GetArgument<std::string>(arguments, 1, ff))
    {
        if (ff.compare("friend") == 0)
            class_ = Game::TargetClass::Friend;
        else if (ff.compare("foe") == 0)
            class_ = Game::TargetClass::Foe;
    }
    GetArgument<int32_t>(arguments, 2, minActivationTime_);
}

void SelectTargetUsingSkill::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    Game::Npc& chr = GetNpc(agent);
    auto& rng = *GetSubsystem<Crypto::Random>();
    chr.VisitInRange(Game::Ranges::Casting, [&](const Game::GameObject& current)
    {
        if (!Game::Is<Game::Actor>(current))
            return Iteration::Continue;

        const Game::Actor& actor = Game::To<Game::Actor>(current);
        if (!Game::TargetClassMatches(chr, class_, actor))
            return Iteration::Continue;
        if (!actor.IsSelectable() || actor.IsDead() || actor.IsUndestroyable())
            return Iteration::Continue;

        if (actor.IsUsingSkillOfType(type_, minActivationTime_))
        {
            if (rng.GetFloat() < 0.3f)
                return Iteration::Continue;
#ifdef DEBUG_AI
            LOG_DEBUG << "Selected " << actor.GetName() << std::endl;
#endif
            entities.push_back(actor.id_);
            return Iteration::Break;
        }
        return Iteration::Continue;
    });
}

}
}
