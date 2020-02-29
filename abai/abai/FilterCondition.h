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

#include "Condition.h"

namespace AI {

class Filter;

namespace Conditions {

// Returns true when the filter selected something
class FilterCondition : public Condition
{
    CONDITON_CLASS(FilterCondition)
private:
    std::shared_ptr<Filter> filter_;
    // Minimum number of selected entities to succeed
    uint32_t min_{ 1 };
public:
    explicit FilterCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&, const Node&) override;
    bool SetFilter(std::shared_ptr<AI::Filter> filter) override;
    const Filter* GetFilter() const override;
    std::string GetFriendlyName() const override;
};

}
}
