#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsAllyHealthCritical : public AI::Condition
{
public:
    CONDITON_FACTORY(IsAllyHealthCritical)

    explicit IsAllyHealthCritical(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
