#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Die : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(Die)
    explicit Die(const ArgumentsType& arguments);
};

}
}
