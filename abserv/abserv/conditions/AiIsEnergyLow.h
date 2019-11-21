#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsEnergyLow : public AI::Condition
{
    CONDITON_CLASS(IsEnergyLow)
public:
    explicit IsEnergyLow(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
