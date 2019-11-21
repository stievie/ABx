#pragma once

#include "Condition.h"

namespace AI {

class Agent;

namespace Conditions {

class IsSelectionAlive : public AI::Condition
{
    CONDITON_CLASS(IsSelectionAlive)
public:
    explicit IsSelectionAlive(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
