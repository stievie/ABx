
#pragma once

#include "SkillAction.h"

namespace AI {
namespace Actions {

class HealOther final : public SkillAction
{
    NODE_CLASS(HealOther)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit HealOther(const ArgumentsType& arguments);
};

}
}
