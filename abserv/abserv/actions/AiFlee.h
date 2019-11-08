#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Flee : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(Flee)
    explicit Flee(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}
