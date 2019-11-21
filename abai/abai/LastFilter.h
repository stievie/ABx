#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class LastFilter : public Filter
{
    FILTER_CLASS(LastFilter)
public:
    explicit LastFilter(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
