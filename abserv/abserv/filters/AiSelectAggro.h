#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAggro : public Filter
{
public:
    FILTER_FACTORY(SelectAggro)
    explicit SelectAggro(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
