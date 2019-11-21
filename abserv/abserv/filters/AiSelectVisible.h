#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectVisible : public Filter
{
    FILTER_CLASS(SelectVisible)
public:
    explicit SelectVisible(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
