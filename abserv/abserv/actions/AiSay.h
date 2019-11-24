#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Say final : public Action
{
    NODE_CLASS(Say)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Say(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}
