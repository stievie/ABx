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

#include <unordered_map>
#include <functional>

namespace sa {

template <typename IndexType, typename ReturnType, typename... _CArgs>
class CallableTable
{
public:
    using FunctionType = std::function<ReturnType(_CArgs...)>;
private:
    std::unordered_map<IndexType, FunctionType> callables_;
public:
    void Add(IndexType index, FunctionType&& f)
    {
        callables_[index] = std::move(f);
    }
    ReturnType Call(IndexType index, _CArgs&& ... _Args)
    {
        assert(Exists(index));
        auto& c = callables_[index];
        return c(std::forward<_CArgs>(_Args)...);
    }
    bool Exists(IndexType index) const
    {
        return callables_.find(index) != callables_.end();
    }
    FunctionType& operator [](IndexType index) {
        assert(Exists(index));
        return callables_[index];
    }
};

}
