#pragma once

#include "Decorator.h"

namespace AI {

class UntilSuccess : public Decorator
{
    NODE_CLASS(UntilSuccess)
public:
    explicit UntilSuccess(const ArgumentsType& arguments) :
        Decorator(arguments)
    { }
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}

