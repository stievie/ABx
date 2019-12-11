#pragma once

#include "SkillAction.h"
#include "../Skill.h"

namespace AI {
namespace Actions {

class UseDamageSkill final : public SkillAction
{
    NODE_CLASS(UseDamageSkill)
private:
    // By default any target type
    Game::SkillTarget targetType_{ Game::SkillTargetTarget };
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit UseDamageSkill(const ArgumentsType& arguments) :
        SkillAction(arguments)
    {
        if (arguments.size() > 0)
        {
            targetType_ = static_cast<Game::SkillTarget>(atoi(arguments[0].c_str()));
        }
    }
};

}
}
