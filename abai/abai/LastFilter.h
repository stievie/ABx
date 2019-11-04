#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class LastFilter : public Filter
{
public:
    FILTER_FACTORY(LastFilter)
    explicit LastFilter(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
