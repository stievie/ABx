#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class IsAllyHealthCritical : public Condition
{
public:
    CONDITON_CLASS(IsAllyHealthCritical)

    explicit IsAllyHealthCritical(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
