#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class GainEnergy final : public Action
{
    NODE_CLASS(GainEnergy)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit GainEnergy(const ArgumentsType& arguments) :
        Action(arguments)
    {
        mustComplete_ = true;
    }
};

}
}
