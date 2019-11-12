#pragma once

#include "Decorator.h"

namespace AI {

class UntilSuccess : public Decorator
{
public:
    NODE_CLASS(UntilSuccess)
    explicit UntilSuccess(const ArgumentsType& arguments) :
        Decorator(arguments)
    { }
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}

