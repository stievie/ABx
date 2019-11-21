#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

// Selects all agents in the Zone
class ZoneFilter : public Filter
{
    FILTER_CLASS(ZoneFilter)
public:
    explicit ZoneFilter(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
