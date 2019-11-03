#pragma once

#include "Decorator.h"

namespace AI {

class Root : public Decorator
{
public:
    NODE_FACTORY(Root)
    explicit Root(const NodeFactoryContext& ctx);
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) override;
};

}
