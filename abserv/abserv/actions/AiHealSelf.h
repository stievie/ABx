#pragma once

#include "SkillAction.h"

namespace AI {
namespace Actions {

class HealSelf final : public SkillAction
{
    NODE_CLASS(HealSelf)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit HealSelf(const ArgumentsType& arguments);
};

}
}
