#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAggro final : public Filter
{
    FILTER_CLASS(SelectAggro)
public:
    explicit SelectAggro(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
