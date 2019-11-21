
#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class HealOther : public Action
{
    NODE_CLASS(HealOther)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit HealOther(const ArgumentsType& arguments);
};

}
}
