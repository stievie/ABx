#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

// Return true when the agent has selected something.
class HasSelection : public Condition
{
public:
    CONDITON_CLASS(HasSelection)
    explicit HasSelection(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    virtual bool Evaluate(Agent& agent, const Node&) override;
};

}

}
