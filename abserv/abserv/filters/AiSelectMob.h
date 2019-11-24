#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

// Select the foe with most other foes around
class SelectMob final : public Filter
{
    FILTER_CLASS(SelectMob)
public:
    explicit SelectMob(const ArgumentsType& arguments) :
        Filter(arguments)
    { }
    void Execute(Agent& agent) override;
};

}
}
