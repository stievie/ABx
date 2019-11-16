#pragma once

#include "Node.h"

namespace AI {

// Do something useful, e.g. attacking mean foes. AKA Leaf.
// There is no factory for this because it's meant to be subclassed.
class Action : public Node, public std::enable_shared_from_this<Action>
{
protected:
    // If true tthis Action is executted until it returns not running without
    // revaluating the tree.
    bool mustComplete_{ false };
    virtual Status DoAction(Agent& agent, uint32_t timeElapsed) = 0;
public:
    explicit Action(const ArgumentsType& arguments);
    ~Action() override;

    bool MustComplete() const { return mustComplete_; }
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
