#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Wander final : public Action
{
    NODE_CLASS(Wander)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Wander(const ArgumentsType& arguments);
};

}
}
