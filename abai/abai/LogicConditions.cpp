#include "stdafx.h"
#include "LogicConditions.h"

namespace AI {
namespace Conditions {

FalseCondition::FalseCondition(const ConditionFactoryContext&) :
    Condition()
{ }

bool FalseCondition::Evaluate(const Agent&)
{
    return false;
}

TrueCondition::TrueCondition(const ConditionFactoryContext&) :
    Condition()
{ }

bool TrueCondition::Evaluate(const Agent&)
{
    return true;
}

AndCondition::AndCondition(const ConditionFactoryContext& ctx) :
    Condition(),
    conditions_(ctx.conditions)
{ }

bool AndCondition::Evaluate(const Agent& agent)
{
    for (auto& condition : conditions_)
    {
        if (!condition->Evaluate(agent))
            return false;
    }
    return true;
}

OrCondition::OrCondition(const ConditionFactoryContext& ctx) :
    Condition(),
    conditions_(ctx.conditions)
{ }

bool OrCondition::Evaluate(const Agent& agent)
{
    for (auto& condition : conditions_)
    {
        if (condition->Evaluate(agent))
            return true;
    }
    return false;
}

NotCondition::NotCondition(const ConditionFactoryContext& ctx) :
    Condition(),
    condition_(ctx.conditions.front())
{ }

bool NotCondition::Evaluate(const Agent& agent)
{
    return !condition_->Evaluate(agent);
}

}
}
