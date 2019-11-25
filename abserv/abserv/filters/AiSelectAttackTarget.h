#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

// Select a target which we will attack
class SelectAttackTarget final : public Filter
{
    FILTER_CLASS(SelectAttackTarget)
public:
    explicit SelectAttackTarget(const ArgumentsType& arguments) :
        Filter(arguments)
    { }

    void Execute(Agent& agent) override;
};

}
}
