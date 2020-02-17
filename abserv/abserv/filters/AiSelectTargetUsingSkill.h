/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <abai/Filter.h>
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
class SelectTargetUsingSkill final : public Filter
{
    FILTER_CLASS(SelectTargetUsingSkill)
private:
    Game::TargetClass class_{ Game::TargetClass::All };
    AB::Entities::SkillType type_{ AB::Entities::SkillTypeSkill };
    // Ignore when activation time is too short
    int32_t minActivationTime_{ 0 };
public:
    explicit SelectTargetUsingSkill(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
