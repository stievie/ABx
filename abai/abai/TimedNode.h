#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include "Action.h"
#include <limits>

namespace AI {

constexpr uint32_t NOT_STARTED = std::numeric_limits<uint32_t>::max();

class Condition;

class TimedNode : public Action
{
protected:
    uint32_t millis_{ 0 };
protected:
    // Called by Action, be we don't use it
    Status DoAction(Agent&, uint32_t) override { return Status::CanNotExecute; }
public:
    explicit TimedNode(const ArgumentsType& arguments);
    ~TimedNode() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
    virtual Node::Status ExecuteStart(Agent& agent, uint32_t timeElapsed);
    virtual Node::Status ExecuteRunning(Agent& agent, uint32_t timeElapsed);
    virtual Node::Status ExecuteExpired(Agent& agent, uint32_t timeElapsed);
};

}
