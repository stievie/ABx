#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Wander : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(Wander)
    explicit Wander(const ArgumentsType& arguments);
};

}
}
