#pragma once

#include "Agent.h"

namespace Game {
class Npc;
}

namespace AI {

class AiAgent : public Agent
{
private:
    Game::Npc& npc_;
public:
    AiAgent(Game::Npc& npc);
    ~AiAgent() override;

    Game::Npc& GetNpc() { return npc_; }
    const Game::Npc& GetNpc() const { return npc_; }

    int selectedSkill_{ -1 };
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
