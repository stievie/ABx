#pragma once

#include "Decorator.h"

namespace AI {

// Decorator which returns only running or failed
class Fail : public Decorator
{
public:
    NODE_FACTORY(Fail)
    explicit Fail(const NodeFactoryContext& ctx);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
