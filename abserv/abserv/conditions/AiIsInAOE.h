#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsInAOE : public AI::Condition
{
public:
    CONDITON_FACTORY(IsInAOE)

    explicit IsInAOE(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
