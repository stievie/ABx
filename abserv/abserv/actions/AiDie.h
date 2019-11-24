#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Die final : public Action
{
    NODE_CLASS(Die)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Die(const ArgumentsType& arguments);
};

}
}
