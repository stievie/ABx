
#pragma once

#include "Action.h"

namespace AI {

class HealOther : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(HealOther)
    explicit HealOther(const ArgumentsType& arguments);
};

}
