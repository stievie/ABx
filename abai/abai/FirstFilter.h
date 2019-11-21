#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class FirstFilter : public Filter
{
    FILTER_CLASS(FirstFilter)
public:
    explicit FirstFilter(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
