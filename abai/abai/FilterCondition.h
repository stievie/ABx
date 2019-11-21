#pragma once

#include "Condition.h"

namespace AI {

class Filter;

namespace Conditions {

// Returns true when the filter selected something
class FilterCondition : public Condition
{
    CONDITON_CLASS(FilterCondition)
private:
    std::shared_ptr<Filter> filter_;
    // Minimum number of selected entitites to succeed
    uint32_t min_{ 1 };
public:
    explicit FilterCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&, const Node&) override;
    bool SetFilter(std::shared_ptr<AI::Filter> filter) override;
};

}
}
