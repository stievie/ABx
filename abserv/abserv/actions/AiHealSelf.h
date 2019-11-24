#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class HealSelf final : public Action
{
    NODE_CLASS(HealSelf)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit HealSelf(const ArgumentsType& arguments);
};

}
}
