#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsMeleeTarget : public AI::Condition
{
    CONDITON_CLASS(IsMeleeTarget)
public:
    explicit IsMeleeTarget(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
