#pragma once

#include "Decorator.h"

namespace AI {

// Decorator that returns either running or finished
class Succeed : public Decorator
{
    NODE_CLASS(Succeed)
public:
    explicit Succeed(const ArgumentsType& arguments);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
