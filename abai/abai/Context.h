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

#include "AiDefines.h"
#include <unordered_map>
#include <tuple>
#include <sa/Iteration.h>

namespace AI {

template <typename... Types>
class Context
{
private:
    template<typename T>
    struct Values
    {
        std::unordered_map<Id, T> values;
    };

    std::tuple<Values<Types>...> values_;

    template <unsigned int Index>
    using GetTypeOfElement = typename std::tuple_element<Index, decltype(values_)>::type;
    template<typename T, unsigned int Index>
    using IsRightElement = std::is_same<GetTypeOfElement<Index>, T>;
    template<typename T, unsigned int Index = 0>
    struct FindElement : public std::conditional_t<
        IsRightElement<T, Index>::value,
        std::integral_constant<decltype(Index), Index>,
        FindElement<T, Index + 1>>
    {};
    template <typename T>
    Values<T>& GetValuesT()
    {
        constexpr auto index = FindElement<Values<T>>::value;
        return std::get<index>(values_);
    }
    template <typename T>
    const Values<T>& GetValuesT() const
    {
        constexpr auto index = FindElement<Values<T>>::value;
        return std::get<index>(values_);
    }
public:
    template <typename T>
    bool Has(Id id) const
    {
        const auto& vals = GetValuesT<T>().values;
        const auto it = vals.find(id);
        return it != vals.end();
    }
    template <typename T>
    T Get(Id id) const
    {
        return GetValuesT<T>().values[id];
    }
    template <typename T>
    T& Get(Id id)
    {
        return GetValuesT<T>().values[id];
    }
    template <typename T>
    const T& Get(Id id) const
    {
        return GetValuesT<T>().values[id];
    }
    template <typename T>
    void Set(Id id, T value)
    {
        GetValuesT<T>().values[id] = value;
    }
    template <typename T>
    void Delete(Id id)
    {
        auto& vals = GetValuesT<T>().values;
        auto it = vals.find(id);
        if (it != vals.end())
            vals.erase(it);
    }
    template <typename T, typename Callback>
    void VisitTypes(const Callback callback) const
    {
        const auto& vals = GetValuesT<T>().values;
        for (const auto& val : vals)
        {
            // Iteration callback(Id id, T value)
            if (callback(val.first, val.second) == Iteration::Break)
                break;
        }
    }
};

}

