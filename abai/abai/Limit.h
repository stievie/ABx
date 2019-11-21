#pragma once

#include "Decorator.h"

namespace AI {

// Decorator which limits the amount of executionn times. If this limit
// is reached it return finished.
class Limit : public Decorator
{
    NODE_CLASS(Limit)
private:
    uint32_t limit_{ 0 };
public:
    explicit Limit(const ArgumentsType& arguments);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
