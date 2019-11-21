#pragma once

#include "Composite.h"

namespace AI {

// A sequence will visit each child in order, starting with the first, and when that
// succeeds will call the second, and so on down the list of children. If any child
// fails it will immediately return failure to the parent. If the last child in the
// sequence succeeds, then the sequence will return success to its parent.
class Sequence : public Composite
{
    NODE_CLASS(Sequence)
public:
    explicit Sequence(const ArgumentsType& arguments);
    ~Sequence() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
