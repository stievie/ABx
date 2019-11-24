#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

// Selectt a target that is in attack state
class SelectTargetAttacking final : public Filter
{
    FILTER_CLASS(SelectTargetAttacking)
public:
    explicit SelectTargetAttacking(const ArgumentsType& arguments) :
      Filter(arguments)
    { }
    void Execute(Agent& agent) override;
};

}
}
