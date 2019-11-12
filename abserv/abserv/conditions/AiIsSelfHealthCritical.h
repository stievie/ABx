#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsSelfHealthCritical : public AI::Condition
{
public:
    CONDITON_CLASS(IsSelfHealthCritical)

    explicit IsSelfHealthCritical(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
