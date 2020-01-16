#pragma once

#include "AiAgent.h"
#include <sa/Noncopyable.h>

namespace Game {

class Npc;

namespace Components {

class AiComp
{
    NON_COPYABLE(AiComp)
private:
    Npc& owner_;
    AI::AiAgent agent_;
    AiComp() = delete;
public:
    explicit AiComp(Npc& owner);
    ~AiComp() = default;

    void Update(uint32_t timeElapsed);
    AI::AiAgent& GetAgent() { return agent_; }
};

}
}
