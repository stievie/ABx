#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SortByDistance final : public Filter
{
    FILTER_CLASS(SortByDistance)
public:
    SortByDistance(const ArgumentsType& arguments) :
        Filter(arguments)
    { }
    void Execute(Agent& agent) override;
};

}
}
