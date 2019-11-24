#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class IsInWeaponRange final : public Condition
{
    CONDITON_CLASS(IsInWeaponRange)
public:
    explicit IsInWeaponRange(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}
