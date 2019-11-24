#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class IsInSkillRange final : public Condition
{
    CONDITON_CLASS(IsInSkillRange)
public:
    explicit IsInSkillRange(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}

}

