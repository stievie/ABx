#pragma once

#include "Condition.h"

namespace AI {

class Filter;

namespace Conditions {

// Returns true when the filter selected something
class FilterCondition : public Condition
{
private:
    std::shared_ptr<Filter> filter_;
public:
    CONDITON_CLASS(FilterCondition)
    explicit FilterCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&) override;
    bool SetFilter(std::shared_ptr<AI::Filter> filter) override;
};

}
}
