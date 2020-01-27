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

#include "Node.h"
#include "Filter.h"
#include "Condition.h"
#include <memory>

namespace AI {

class Registry
{
protected:
    using NodeFactoryRegistry = FactoryRegistry<std::string, Node>;
    using FilterFactoryRegistry = FactoryRegistry<std::string, Filter>;
    using ConditionFactoryRegistry = FactoryRegistry<std::string, Condition>;

    NodeFactoryRegistry nodeFactory_;
    FilterFactoryRegistry filterFactory_;
    ConditionFactoryRegistry conditionFactory_;
public:
    Registry();
    virtual ~Registry();
    virtual void Initialize();

    bool RegisterNodeFactory(const std::string& name, const NodeFactory& factory);
    bool UnregisterNodeFactory(const std::string& name);
    bool RegisterFilterFactory(const std::string& name, const FilterFactory& factory);
    bool UnregisterFilterFactory(const std::string& name);
    bool RegisterConditionFactory(const std::string& name, const ConditionFactory& factory);
    bool UnregisterConditionFactory(const std::string& name);

    std::shared_ptr<Node> CreateNode(const std::string& nodeType, const ArgumentsType& arguments);
    std::shared_ptr<Filter> CreateFilter(const std::string& filterType, const ArgumentsType& arguments);
    std::shared_ptr<Condition> CreateCondition(const std::string& conditionType, const ArgumentsType& arguments);
};

}
