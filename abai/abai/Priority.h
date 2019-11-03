#pragma once

#include "Composite.h"

namespace AI {

// Fallback nodes are used to find and execute the first child that does not fail.
// A fallback node will return immediately with a status code of success or running
// when one of its children returns success or running. The children are ticked in
// order of importance, from left to right.
class Priority : public Composite
{
private:
    Nodes::iterator it_;
public:
    NODE_FACTORY(Priority)
    explicit Priority(const NodeFactoryContext& ctx);
    ~Priority() override;
    void Initialize() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
