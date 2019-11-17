#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsMeleeTarget : public AI::Condition
{
public:
    CONDITON_CLASS(IsMeleeTarget)

    explicit IsMeleeTarget(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
