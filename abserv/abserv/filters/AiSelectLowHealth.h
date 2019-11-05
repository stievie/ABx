#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectLowHealth : public Filter
{
public:
    FILTER_FACTORY(SelectLowHealth)
    explicit SelectLowHealth(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
