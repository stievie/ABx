#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectLowHealth : public Filter
{
    FILTER_CLASS(SelectLowHealth)
public:
    explicit SelectLowHealth(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
