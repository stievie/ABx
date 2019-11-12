#pragma once

#include "Decorator.h"

namespace AI {

// Decorator which returns only running or failed
class Fail : public Decorator
{
public:
    NODE_CLASS(Fail)
    explicit Fail(const ArgumentsType& arguments);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
