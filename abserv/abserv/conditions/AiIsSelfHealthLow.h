#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsSelfHealthLow : public AI::Condition
{
public:
    CONDITON_CLASS(IsSelfHealthLow)

    explicit IsSelfHealthLow(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
