#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class HealSelf : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(HealSelf)
    explicit HealSelf(const ArgumentsType& arguments);
};

}
}
