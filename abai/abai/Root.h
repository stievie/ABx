#pragma once

#include "Decorator.h"

namespace AI {

class Root : public Decorator
{
public:
    Root();
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
