#pragma once

#include "Decorator.h"

namespace AI {

// Invert the result of the child
class Inverter : public Decorator
{
    NODE_CLASS(Inverter)
public:
    explicit Inverter(const ArgumentsType& arguments);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
