#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class GoHome : public Action
{
    NODE_CLASS(GoHome)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit GoHome(const ArgumentsType& arguments);
};

}
}
