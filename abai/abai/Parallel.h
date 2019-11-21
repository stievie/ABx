#pragma once

#include "Composite.h"

namespace AI {

// Executes all children no matter what they return
class Parallel : public Composite
{
    NODE_CLASS(Parallel)
public:
    explicit Parallel(const ArgumentsType& arguments);
    ~Parallel() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
