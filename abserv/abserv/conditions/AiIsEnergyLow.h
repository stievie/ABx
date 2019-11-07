#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsEnergyLow : public AI::Condition
{
public:
    CONDITON_FACTORY(IsEnergyLow)

    explicit IsEnergyLow(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
