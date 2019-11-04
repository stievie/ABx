#include "stdafx.h"
#include "LogicConditions.h"

namespace AI {
namespace Conditions {

FalseCondition::FalseCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
}

bool FalseCondition::Evaluate(Agent&)
{
    return false;
}

TrueCondition::TrueCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
}

bool TrueCondition::Evaluate(Agent&)
{
    return true;
}

AndCondition::AndCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
}

bool AndCondition::AddCondition(std::shared_ptr<Condition> condition)
{
    conditions_.push_back(condition);
    return true;
}

bool AndCondition::Evaluate(Agent& agent)
{
    for (auto& condition : conditions_)
    {
        if (!condition->Evaluate(agent))
            return false;
    }
    return true;
}

OrCondition::OrCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
}

bool OrCondition::AddCondition(std::shared_ptr<Condition> condition)
{
    conditions_.push_back(condition);
    return true;
}

bool OrCondition::Evaluate(Agent& agent)
{
    for (auto& condition : conditions_)
    {
        if (condition->Evaluate(agent))
            return true;
    }
    return false;
}

NotCondition::NotCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
}

bool NotCondition::AddCondition(std::shared_ptr<Condition> condition)
{
    if (condition)
        return false;
    condition_ = condition;
    return true;
}

bool NotCondition::Evaluate(Agent& agent)
{
    return !condition_->Evaluate(agent);
}

}
}
