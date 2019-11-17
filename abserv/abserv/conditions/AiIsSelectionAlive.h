#pragma once

#include "Condition.h"

namespace AI {

class Agent;

namespace Conditions {

class IsSelectionAlive : public AI::Condition
{
public:
    CONDITON_CLASS(IsSelectionAlive)

    explicit IsSelectionAlive(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
