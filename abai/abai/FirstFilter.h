#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class FirstFilter : public Filter
{
public:
    FILTER_FACTORY(FirstFilter)
    explicit FirstFilter(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
