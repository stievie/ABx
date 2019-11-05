#pragma once

#include "AiAgent.h"

namespace Game {

class Npc;

namespace Components {

class AiComp
{
private:
    Npc& owner_;
    AI::AiAgent agent_;
public:
    AiComp() = delete;
    explicit AiComp(Npc& owner);
    // non-copyable
    AiComp(const AiComp&) = delete;
    AiComp& operator=(const AiComp&) = delete;
    ~AiComp() = default;

    void Update(uint32_t timeElapsed);
    AI::AiAgent& GetAgent() { return agent_; }
};

}
}
