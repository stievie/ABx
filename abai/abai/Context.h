#pragma once

#include "AiDefines.h"
#include <unordered_map>
#include <tuple>

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
public:
    template <typename T>
    bool Exists(Id id)
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
};

}

