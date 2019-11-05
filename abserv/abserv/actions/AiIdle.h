#pragma once

#include "TimedNode.h"

namespace AI {

class Idle : public TimedNode
{
public:
    NODE_FACTORY(Idle)
    explicit Idle(const ArgumentsType& arguments) :
        TimedNode(arguments)
    { }
    Node::Status ExecuteStart(Agent& agent, uint32_t timeElapsed) override;
};

}
