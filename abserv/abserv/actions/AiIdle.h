#pragma once

#include "TimedNode.h"

namespace AI {
namespace Actions {

class Idle final : public TimedNode
{
    NODE_CLASS(Idle)
public:
    explicit Idle(const ArgumentsType& arguments) :
        TimedNode(arguments)
    { }
    Node::Status ExecuteStart(Agent& agent, uint32_t timeElapsed) override;
};

}
}
