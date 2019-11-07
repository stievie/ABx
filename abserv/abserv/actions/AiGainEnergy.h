#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class GainEnergy : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(GainEnergy)
    explicit GainEnergy(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}
