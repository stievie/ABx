#include "stdafx.h"
#include "FilterCondition.h"
#include "Filter.h"

namespace AI {
namespace Conditions {

FilterCondition::FilterCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
    if (arguments.size() != 0)
        min_ = static_cast<uint32_t>(atoi(arguments[0].c_str()));
}

bool FilterCondition::Evaluate(Agent& agent, const Node&)
{
    // If there is a filter assigned execute it firs, otherwise just see if the
    // Agent has something selected.
    if (filter_)
        filter_->Execute(agent);
    return agent.filteredAgents_.size() >= min_;
}

bool FilterCondition::SetFilter(std::shared_ptr<AI::Filter> filter)
{
    if (!filter)
        return false;
    filter_ = filter;
    return true;
}

}
}
