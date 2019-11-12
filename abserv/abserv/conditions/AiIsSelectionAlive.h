#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsSelectionAlive : public AI::Condition
{
public:
    CONDITON_CLASS(IsSelectionAlive)

    explicit IsSelectionAlive(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&) override;
};

}
}
