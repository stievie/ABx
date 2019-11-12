#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectVisible : public Filter
{
public:
    FILTER_CLASS(SelectVisible)
    explicit SelectVisible(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
