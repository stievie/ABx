#include "stdafx.h"
#include "FilterCondition.h"
#include "Filter.h"

namespace AI {
namespace Conditions {

FilterCondition::FilterCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool FilterCondition::Evaluate(Agent& agent, const Node&)
{
    // If there is a filter assigned execute it firs, otherwise just see if the
    // Agent has something selected.
    if (filter_)
        filter_->Execute(agent);
    return agent.filteredAgents_.size() != 0;
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
