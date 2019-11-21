#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsSelfHealthLow : public AI::Condition
{
    CONDITON_CLASS(IsSelfHealthLow)
public:
    explicit IsSelfHealthLow(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
