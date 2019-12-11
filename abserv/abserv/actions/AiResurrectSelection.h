#pragma once

#include "SkillAction.h"

namespace AI {
namespace Actions {

class ResurrectSelection final : public SkillAction
{
    NODE_CLASS(ResurrectSelection)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit ResurrectSelection(const ArgumentsType& arguments) :
        SkillAction(arguments)
    { }
};

}
}
