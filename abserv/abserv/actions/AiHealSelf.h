#pragma once

#include "Action.h"

namespace AI {

class HealSelf : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(HealSelf)
    explicit HealSelf(const ArgumentsType& arguments);
};

}
