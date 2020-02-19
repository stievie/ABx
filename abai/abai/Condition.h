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

#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <functional>
#include <sa/Iteration.h>
#include "Factory.h"
#include <CleanupNs.h>

namespace AI {

class Agent;
class Condition;
class Filter;
class Node;

using ConditionFactory = AbstractFactory<Condition>;

#define CONDITON_CLASS(ConditionName)                                                    \
public:                                                                                  \
    class Factory : public ConditionFactory                                              \
    {                                                                                    \
    public:                                                                              \
        std::shared_ptr<Condition> Create(const ArgumentsType& arguments) const override \
        {                                                                                \
            std::shared_ptr<Condition> res = std::make_shared<ConditionName>(arguments); \
            return res;                                                                  \
        }                                                                                \
    };                                                                                   \
    static const Factory& GetFactory()                                                   \
    {                                                                                    \
        static Factory sFactory;                                                         \
        return sFactory;                                                                 \
    }                                                                                    \
    ConditionName(const ConditionName&) = delete;                                        \
    ConditionName& operator=(const ConditionName&) = delete;                             \
    ConditionName(ConditionName&&) = delete;                                             \
    ConditionName& operator=(ConditionName&&) = delete;                                  \
    const char* GetClassName() const override { return ABAI_STRINGIFY(ConditionName); }

class Condition
{
protected:
    std::string name_;
public:
    explicit Condition(const ArgumentsType& arguments);
    virtual ~Condition();

    virtual const char* GetClassName() const = 0;
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }
    virtual std::string GetFriendlyName() const { return GetClassName(); }

    virtual bool AddCondition(std::shared_ptr<Condition>);
    // Visit first level conditions (not a whole condition tree)
    virtual void VisitConditions(const std::function<Iteration(const Condition&)>&) const { }
    virtual bool SetFilter(std::shared_ptr<Filter>);
    virtual const Filter* GetFilter() const { return nullptr; }
    /// Evaluate the condition
    /// @param agent
    /// @param node The calling node
    virtual bool Evaluate(Agent& agent, const Node& node) = 0;
};

}
