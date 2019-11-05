#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectDeadAllies : public Filter
{
public:
    FILTER_FACTORY(SelectDeadAllies)
    explicit SelectDeadAllies(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
