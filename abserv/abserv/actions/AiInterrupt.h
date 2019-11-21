#pragma once

#include "Action.h"
#include <AB/Entities/Skill.h>

namespace AI {
namespace Actions {

class Interrupt : public Action
{
    NODE_CLASS(Interrupt)
private:
    AB::Entities::SkillType type_{ AB::Entities::SkillTypeSpell };
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Interrupt(const ArgumentsType& arguments);
};

}
}
