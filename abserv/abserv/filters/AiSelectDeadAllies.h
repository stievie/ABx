#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectDeadAllies : public Filter
{
    FILTER_CLASS(SelectDeadAllies)
public:
    explicit SelectDeadAllies(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
