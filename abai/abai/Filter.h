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

#include "Agent.h"
#include "Factory.h"
#include <CleanupNs.h>
#include <sa/Compiler.h>

namespace AI {

class Filter;

using FilterFactory = AbstractFactory<Filter>;

#define FILTER_CLASS(FilterName)                                                      \
public:                                                                               \
    class Factory final : public FilterFactory                                        \
    {                                                                                 \
    public:                                                                           \
        std::shared_ptr<Filter> Create(const ArgumentsType& arguments) const override \
        {                                                                             \
            std::shared_ptr<Filter> res = std::make_shared<FilterName>(arguments);    \
            return res;                                                               \
        }                                                                             \
    };                                                                                \
    static const Factory& GetFactory()                                                \
    {                                                                                 \
        static Factory sFactory;                                                      \
        return sFactory;                                                              \
    }                                                                                 \
private:                                                                              \
    FilterName(const FilterName&) = delete;                                           \
    FilterName& operator=(const FilterName&) = delete;                                \
    FilterName(FilterName&&) = delete;                                                \
    FilterName& operator=(FilterName&&) = delete;                                     \
    const char* GetClassName() const override { return ABAI_STRINGIFY(FilterName); }

class SA_NOVTABLE Filter
{
protected:
    std::string name_;
    explicit Filter(const ArgumentsType& arguments);
    AgentIds& GetFiltered(Agent& agent)
    {
        return agent.filteredAgents_;
    }
public:
    virtual ~Filter();

    virtual const char* GetClassName() const = 0;
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& value) { name_ = value; }

    virtual void Execute(Agent&) = 0;
};

}
