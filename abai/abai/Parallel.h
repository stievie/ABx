#pragma once

#include "Composite.h"

namespace AI {

// Executes all children no matter what they return
class Parallel : public Composite
{
public:
    NODE_FACTORY(Parallel)
    explicit Parallel(const NodeFactoryContext& ctx);
    ~Parallel() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
