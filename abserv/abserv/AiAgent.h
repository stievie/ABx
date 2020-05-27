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

#pragma once

#include <abai/Agent.h>
#include <absmath/Vector3.h>
#include <sa/StrongType.h>

namespace Game {
class Npc;
}

namespace AI {

using SkillIndexType = sa::StrongType<int, struct SkillIndexTag>;
using TargetIdType = sa::StrongType<Id, struct TargetTag>;
using DistanceType = sa::StrongType<float, struct DistanceTag>;

using AiAgentContext = Context<
    SkillIndexType,
    TargetIdType,
    DistanceType,
    Math::Vector3
>;

class AiAgent final : public Agent
{
private:
    Game::Npc& npc_;
public:
    AiAgent(Game::Npc& npc);
    ~AiAgent() override;

    Game::Npc& GetNpc() { return npc_; }
    const Game::Npc& GetNpc() const { return npc_; }

    int selectedSkill_{ -1 };
    AiAgentContext aiContext_;
};

inline Game::Npc& GetNpc(Agent& agent)
{
    return static_cast<AiAgent&>(agent).GetNpc();
}
inline const Game::Npc& GetNpc(const Agent& agent)
{
    return static_cast<const AiAgent&>(agent).GetNpc();
}
inline AiAgent& GetAgent(Agent& agent)
{
    return static_cast<AiAgent&>(agent);
}
inline const AiAgent& GetAgent(const Agent& agent)
{
    return static_cast<const AiAgent&>(agent);
}

}
