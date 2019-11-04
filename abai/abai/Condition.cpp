#include "stdafx.h"
#include "Condition.h"

namespace AI {

Condition::Condition(const ArgumentsType&)
{ }

Condition::~Condition() = default;

bool Condition::AddCondition(std::shared_ptr<Condition>)
{
    return false;
}

bool Condition::SetFilter(std::shared_ptr<Filter>)
{
    return false;
}

}
