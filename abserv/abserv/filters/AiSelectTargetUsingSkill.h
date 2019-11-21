#pragma once

#include "Filter.h"
#include <AB/Entities/Skill.h>
#include "../Actor.h"

namespace AI {
namespace Filters {

// Select foe currently using a skill
//
// 3 Arguments:
//  1. Optional friend|foe(default)|all
//  2. Skill type as defined in Entities/Skill.h and /scripts/includes/skill_consts.lua
//  3. Min skill activation time (optional)
class SelectTargetUsingSkill : public Filter
{
    FILTER_CLASS(SelectTargetUsingSkill)
private:
    Game::TargetClass class_{ Game::TargetClass::Foe };
    AB::Entities::SkillType type_{ AB::Entities::SkillTypeSkill };
    // Ignore when activation time is too short
    int32_t minActivationTime_{ 0 };
public:
    explicit SelectTargetUsingSkill(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
