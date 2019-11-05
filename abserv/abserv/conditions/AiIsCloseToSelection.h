#pragma once

#include "Condition.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

class IsCloseToSelection : public AI::Condition
{
private:
    float distance_;
public:
    CONDITON_FACTORY(IsCloseToSelection)

    explicit IsCloseToSelection(const ArgumentsType& arguments);
    bool Evaluate(AI::Agent&) override;
};

}
}
