#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAggro : public Filter
{
public:
    FILTER_CLASS(SelectAggro)
    explicit SelectAggro(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
