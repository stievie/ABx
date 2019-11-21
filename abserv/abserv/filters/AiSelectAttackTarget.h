#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAttackTarget : public Filter
{
    FILTER_CLASS(SelectAttackTarget)
public:
    explicit SelectAttackTarget(const ArgumentsType& arguments) :
        Filter(arguments)
    { }

    void Execute(Agent& agent) override;
};

}
}
