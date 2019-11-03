#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include "Node.h"
#include <limits>

namespace AI {

constexpr uint32_t NOT_STARTED = std::numeric_limits<uint32_t>::max();

class Condition;

class TimedNode : public Node
{
protected:
    uint32_t millis_;
    uint32_t timer_{ NOT_STARTED };
public:
    TimedNode(const NodeFactoryContext& ctx);
    ~TimedNode() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
    virtual Node::Status ExecuteStart(Agent& agent, uint32_t timeElapsed);
    virtual Node::Status ExecuteRunning(Agent& agent, uint32_t timeElapsed);
    virtual Node::Status ExecuteExpired(Agent& agent, uint32_t timeElapsed);
};

}
