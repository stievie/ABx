#pragma once

#include "Decorator.h"

namespace AI {

// Run the child until it fails. Then it retutrns Finished.
class UntilFail : public Decorator
{
    NODE_CLASS(UntilFail)
public:
    explicit UntilFail(const ArgumentsType& arguments) :
        Decorator(arguments)
    { }
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
