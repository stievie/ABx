#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsAttacked : public AI::Condition
{
public:
    CONDITON_FACTORY(IsAttacked)

    explicit IsAttacked(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
