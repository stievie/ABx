#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsAttacked final : public AI::Condition
{
    CONDITON_CLASS(IsAttacked)
public:
    explicit IsAttacked(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
