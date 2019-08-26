#pragma once

#include <functional>
#include <unordered_map>
#include <type_traits>
#include <tuple>
#include <vector>

namespace sa {

typedef size_t event_t;

template <typename... Types>
class Events
{
private:
    /// All events with exactly one signature
    template <typename T>
    struct _Events
    {
        std::unordered_map<event_t, std::vector<std::function<T>>> events_;
    };
    std::tuple<_Events<Types>...> events_;

    template <unsigned int Index>
    using GetTypeOfElement = typename std::tuple_element<Index, decltype(events_)>::type;
    template<typename T, unsigned int Index>
    using IsRightElement = std::is_same<GetTypeOfElement<Index>, T>;
    template<typename T, unsigned int Index = 0>
    struct FindElement : public std::conditional_t<
        IsRightElement<T, Index>::value,
        std::integral_constant<decltype(Index), Index>,
        FindElement<T, Index + 1>>
    {};
    template <typename T>
    _Events<T>& GetEventsT()
    {
        constexpr auto index = FindElement<_Events<T>>::value;
        return std::get<index>(events_);
    }
public:
    /// Is used with std::bind()
    template <typename Func>
    void Subscribe(event_t index, std::function<Func>&& func)
    {
        GetEventsT<Func>().events_[index].push_back(std::move(func));
    }
    /// Is used for everything else that looks like a callable, e.g. a Lambda
    template <typename Func>
    void Subscribe(event_t index, Func func)
    {
        GetEventsT<Func>().events_[index].push_back(std::function<Func>(func));
    }

    /// Calls the first subscriber and returns the result
    template <typename Func, typename... _CArgs>
    auto CallOne(event_t index, _CArgs&& ... _Args) -> typename std::invoke_result<Func, _CArgs...>::type
    {
        using ResultType = typename std::invoke_result<Func, _CArgs...>::type;
        static constexpr auto isVoid = std::is_same_v<ResultType, void>;

        auto& events = GetEventsT<Func>();
        const auto it = events.events_.find(index);
        if (it == events.events_.end() || (*it).second.size() == 0)
        {
            // Index not found, return nothing (if void), some default value
            if constexpr(isVoid)
                return;
            else
                return ResultType{};
            // or even better throw an exception
        }
        return (*it).second.front()(std::forward<_CArgs>(_Args)...);
    }

    /// Calls all subscibers and returns a std::vector of results or void
    template <typename Func, typename... _CArgs>
    auto CallAll(event_t index, _CArgs&& ... _Args)
    {
        using ResultType = typename std::invoke_result<Func, _CArgs...>::type;
        static constexpr auto isVoid = std::is_same_v<ResultType, void>;
        auto& events = GetEventsT<Func>();
        const auto it = events.events_.find(index);
        if (it == events.events_.end() || (*it).second.size() == 0)
        {
            // Index not found, return nothing (if void), some default value
            if constexpr(isVoid)
                return;
            else
                return std::vector<ResultType>();
            // or even better throw an exception
        }

        if constexpr(isVoid)
        {
            for (const auto& fun : (*it).second)
                fun(std::forward<_CArgs>(_Args)...);
        }
        else
        {
            std::vector<ResultType> result;
            result.reserve((*it).second.size());
            for (const auto& fun : (*it).second)
                result.push_back(fun(std::forward<_CArgs>(_Args)...));
            return result;
        }
    }
};

}
