#pragma once

#include "Decorator.h"

namespace AI {

// Decorator which returns only running or failed
class Fail : public Decorator
{
    NODE_CLASS(Fail)
public:
    explicit Fail(const ArgumentsType& arguments);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
