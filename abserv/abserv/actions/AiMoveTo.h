#pragma once

#include "Action.h"

namespace AI {

class MoveTo : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(MoveTo)
    explicit MoveTo(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}

