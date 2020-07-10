/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "LogicConditions.h"
#include "Node.h"
#include <sstream>
#include <sa/StringTempl.h>

namespace AI {
namespace Conditions {

FalseCondition::FalseCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool FalseCondition::Evaluate(Agent&, const Node&)
{
    return false;
}

std::string FalseCondition::GetFriendlyName() const
{
    return "False";
}

TrueCondition::TrueCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool TrueCondition::Evaluate(Agent&, const Node&)
{
    return true;
}

std::string TrueCondition::GetFriendlyName() const
{
    return "True";
}

AndCondition::AndCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool AndCondition::AddCondition(std::shared_ptr<Condition> condition)
{
    conditions_.push_back(condition);
    return true;
}

std::string AndCondition::GetFriendlyName() const
{
    std::vector<std::string> conds;
    VisitConditions([&](const Condition& current)
    {
        conds.push_back(current.GetFriendlyName());
        return Iteration::Continue;
    });
    return sa::CombineString(conds, std::string(" And "));
}

bool AndCondition::Evaluate(Agent& agent, const Node& node)
{
    for (auto& condition : conditions_)
    {
        if (!condition->Evaluate(agent, node))
            return false;
    }
    return true;
}

void AndCondition::VisitConditions(const std::function<Iteration(const Condition&)>& callback) const
{
    for (const auto& c : conditions_)
        if (callback(*c) == Iteration::Break)
            break;
}

OrCondition::OrCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool OrCondition::AddCondition(std::shared_ptr<Condition> condition)
{
    conditions_.push_back(condition);
    return true;
}

bool OrCondition::Evaluate(Agent& agent, const Node& node)
{
    for (auto& condition : conditions_)
    {
        if (condition->Evaluate(agent, node))
            return true;
    }
    return false;
}

std::string OrCondition::GetFriendlyName() const
{
    std::vector<std::string> conds;
    VisitConditions([&](const Condition& current)
    {
        conds.push_back(current.GetFriendlyName());
        return Iteration::Continue;
    });
    return sa::CombineString(conds, std::string(" Or "));
}

void OrCondition::VisitConditions(const std::function<Iteration(const Condition&)>& callback) const
{
    for (const auto& c : conditions_)
        if (callback(*c) == Iteration::Break)
            break;
}

NotCondition::NotCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{ }

bool NotCondition::AddCondition(std::shared_ptr<Condition> condition)
{
    if (!condition)
        return false;
    condition_ = condition;
    return true;
}

bool NotCondition::Evaluate(Agent& agent, const Node& node)
{
    // A Not condition must have a condition assigned.
    ASSERT(condition_);
    return !condition_->Evaluate(agent, node);
}

std::string NotCondition::GetFriendlyName() const
{
    if (!condition_)
        return "";
    return "Not " + condition_->GetFriendlyName();
}

void NotCondition::VisitConditions(const std::function<Iteration(const Condition&)>& callback) const
{
    if (condition_)
        callback(*condition_);
}

}
}
