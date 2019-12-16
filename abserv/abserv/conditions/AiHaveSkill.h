#pragma once

#include "Condition.h"
#include "../Skill.h"

namespace AI {
namespace Conditions {

class HaveSkill final : public Condition
{
    CONDITON_CLASS(HaveSkill)
private:
    Game::SkillEffect effect_{ Game::SkillEffectNone };
    Game::SkillEffectTarget effectTarget_{ Game::SkillTargetNone };
public:
    explicit HaveSkill(const ArgumentsType& arguments);
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}
