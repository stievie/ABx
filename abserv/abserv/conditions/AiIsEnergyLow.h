#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsEnergyLow final : public Condition
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
