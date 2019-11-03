#pragma once

#include "Node.h"

namespace AI {

// Do something useful, e.g. attacking mean foes. AKA Leaf
class Action : public Node
{
protected:
    virtual Status DoAction(Agent& agent, uint32_t timeElapsed) = 0;
public:
    explicit Action(const NodeFactoryContext& ctx);
    ~Action() override;

    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
