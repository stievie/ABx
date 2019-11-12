
#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class HealOther : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(HealOther)
    explicit HealOther(const ArgumentsType& arguments);
};

}
}
