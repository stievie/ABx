#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Say : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(Say)
    explicit Say(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}
