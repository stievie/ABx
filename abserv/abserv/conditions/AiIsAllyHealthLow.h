#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsAllyHealthLow : public AI::Condition
{
    CONDITON_CLASS(IsAllyHealthLow)
public:
    explicit IsAllyHealthLow(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
