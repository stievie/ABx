#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsSelfHealthCritical : public AI::Condition
{
    CONDITON_CLASS(IsSelfHealthCritical)
public:
    explicit IsSelfHealthCritical(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
