#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAttackTarget : public Filter
{
public:
    FILTER_FACTORY(SelectAttackTarget)
    explicit SelectAttackTarget(const ArgumentsType& arguments) :
        Filter(arguments)
    { }

    void Execute(Agent& agent) override;
};

}
}
