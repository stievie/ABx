#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class IsAllyHealthCritical : public Condition
{
    CONDITON_CLASS(IsAllyHealthCritical)
public:
    explicit IsAllyHealthCritical(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
