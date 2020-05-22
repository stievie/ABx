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


#include "AiGoHome.h"
#include "../Npc.h"
#include "../AiAgent.h"
#include <abshared/Mechanic.h>

//#define DEBUG_AI

namespace AI {
namespace Actions {

Node::Status GoHome::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);
    if (npc.IsImmobilized())
        return Status::Failed;
    const Math::Vector3& home = npc.GetHomePos();
    if (home.Equals(Math::Vector3::Zero))
        return Status::Failed;

    if (home.Equals(npc.GetPosition(), Game::AT_POSITION_THRESHOLD))
        return Status::Finished;

    if (IsCurrentAction(agent))
        return Status::Running;

    if (npc.GotoHomePos())
    {
#ifdef DEBUG_AI
        LOG_DEBUG << npc.GetName() << " goes home from " << npc.GetPosition().ToString() <<
                     " to " << home.ToString() << std::endl;
#endif
        return Status::Running;
    }
    return Status::Finished;
}

GoHome::GoHome(const ArgumentsType& arguments) :
    Action(arguments)
{ }

}
}
