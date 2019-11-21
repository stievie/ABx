#pragma once

#include "Composite.h"

namespace AI {

// Fallback nodes are used to find and execute the first child that does not fail.
// A fallback node will return immediately with a status code of success or running
// when one of its children returns success or running. The children are ticked in
// order of importance, from left to right.
// Order or importance is top to bottom
class Priority : public Composite
{
    NODE_CLASS(Priority)
public:
    explicit Priority(const ArgumentsType& arguments);
    ~Priority() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
