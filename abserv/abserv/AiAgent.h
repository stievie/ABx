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
};

inline Game::Npc& GetNpc(Agent& agent)
{
    return static_cast<AiAgent&>(agent).GetNpc();
}

}
