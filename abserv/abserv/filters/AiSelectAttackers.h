#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAttackers : public Filter
{
public:
    FILTER_CLASS(SelectAttackers)
    explicit SelectAttackers(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
