#pragma once

#include "Decorator.h"

namespace AI {

// A repeater will reprocess its child node each time its child returns a result.
// These are often used at the very base of the tree, to make the tree to run
// continuously.
class Repeater : public Decorator
{
public:
    NODE_FACTORY(Repeater)
    explicit Repeater(const ArgumentsType& arguments) :
        Decorator(arguments)
    { }
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
