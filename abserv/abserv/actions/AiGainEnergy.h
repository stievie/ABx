#pragma once

#include "SkillAction.h"

namespace AI {
namespace Actions {

class GainEnergy final : public SkillAction
{
    NODE_CLASS(GainEnergy)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit GainEnergy(const ArgumentsType& arguments) :
        SkillAction(arguments)
    { }
};

}
}
