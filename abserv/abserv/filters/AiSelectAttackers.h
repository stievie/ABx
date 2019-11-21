#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectAttackers : public Filter
{
    FILTER_CLASS(SelectAttackers)
public:
    explicit SelectAttackers(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
