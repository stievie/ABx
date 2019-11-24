#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsInAOE final : public Condition
{
    CONDITON_CLASS(IsInAOE)
public:
    explicit IsInAOE(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
