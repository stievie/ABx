#pragma once

#include "Action.h"

namespace AI {

class ResurrectSelection : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(ResurrectSelection)
    explicit ResurrectSelection(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
