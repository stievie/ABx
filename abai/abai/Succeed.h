#pragma once

#include "Decorator.h"

namespace AI {

// Decorator that returns either running or finished
class Succeed : public Decorator
{
public:
    NODE_FACTORY(Succeed)
    Succeed(const NodeFactoryContext& ctx);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
