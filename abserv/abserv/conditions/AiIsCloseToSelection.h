#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsCloseToSelection : public AI::Condition
{
    CONDITON_CLASS(IsCloseToSelection)
private:
    float distance_;
public:
    explicit IsCloseToSelection(const ArgumentsType& arguments);
    bool Evaluate(AI::Agent&, const Node&) override;
};

}
}
