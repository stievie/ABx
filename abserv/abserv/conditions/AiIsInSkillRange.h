#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class IsInSkillRange : public Condition
{
public:
    CONDITON_CLASS(IsInSkillRange)
    explicit IsInSkillRange(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}

}

