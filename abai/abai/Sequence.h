#pragma once

#include "Composite.h"

namespace AI {

// The Sequence ticks each child node in order.
// If a child fails or runs, the sequence returns the same status.
// In the next tick, it will try to run each child in order again.
// If all children succeeds, only then does the sequence succeed.
class Sequence : public Composite
{
private:
    Nodes::iterator it_;
protected:
    void Initialize() override;
public:
    NODE_FACTORY(Sequence)
    explicit Sequence(const ArgumentsType& arguments);
    ~Sequence() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
