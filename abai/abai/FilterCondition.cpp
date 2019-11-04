#include "stdafx.h"
#include "FilterCondition.h"
#include "Filter.h"

namespace AI {
namespace Conditions {

FilterCondition::FilterCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool FilterCondition::Evaluate(Agent& agent)
{
    if (!filter_)
        return false;
    filter_->Execute(agent);
    return agent.filteredAgents_.size() != 0;
}

bool FilterCondition::SetFilter(std::shared_ptr<AI::Filter> filter)
{
    filter_ = filter;
    return true;
}

}
}
