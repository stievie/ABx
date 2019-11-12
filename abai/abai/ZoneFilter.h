#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

// Selects all agents in the Zone
class ZoneFilter : public Filter
{
public:
    FILTER_CLASS(ZoneFilter)
    explicit ZoneFilter(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
