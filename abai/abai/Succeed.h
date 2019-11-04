#pragma once

#include "Decorator.h"

namespace AI {

// Decorator that returns either running or finished
class Succeed : public Decorator
{
public:
    NODE_FACTORY(Succeed)
    explicit Succeed(const ArgumentsType& arguments);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
