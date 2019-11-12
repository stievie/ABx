#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class GoHome : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(GoHome)
    explicit GoHome(const ArgumentsType& arguments);
};

}
}
