#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class LastFilter : public Filter
{
public:
    FILTER_FACTORY(LastFilter)
    void Execute(Agent& agent) override;
};

}
}
