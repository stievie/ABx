#pragma once

#include <functional>
#include <unordered_map>
#include <type_traits>
#include <tuple>
#include <map>
#ifdef SA_EVENTS_ASYNC
#include <future>
#endif

// Define SA_EVENTS_ASYNC to get the Async... methods. On *NIX you must link against
// pthread. Handle with care and know what you are doing.

namespace sa {

// Events are identified by it's function signature and the event_t value. So events with the same
// ID but a different signature are distinct.

/// A unique value identifying an event, I use a string hash therefore size_t but can be anything that
/// an std::unordered_map takes as key.
typedef size_t event_t;

template <typename... Signatures>
class Events
{
private:
    /// All events with exactly one signature
    template <typename T>
    struct _Events
    {
        std::unordered_map<event_t, std::map<size_t, std::function<T>>> events_;
    };
    std::tuple<_Events<Signatures>...> events_;

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
    size_t indices_{ 0 };
    size_t NewIndex() { return ++indices_; }
public:
    /// Is used with std::bind()
    template <typename Signature>
    size_t Subscribe(event_t id, std::function<Signature>&& func)
    {
        auto& events = GetEventsT<Signature>().events_[id];
        size_t index = NewIndex();
        events.emplace(index, std::move(func));
        return index;
    }
    /// Is used for everything else that looks like a callable, e.g. a Lambda
    template <typename Signature>
    size_t Subscribe(event_t id, Signature&& func)
    {
        return Subscribe<Signature>(id, std::function<Signature>(std::move(func)));
    }
    template <typename Signature>
    void Unsubscribe(event_t id, size_t index)
    {
        auto& events = GetEventsT<Signature>();
        const auto it = events.events_.find(id);
        if (it == events.events_.end())
            return;

        auto& eventsMap = (*it).second;
        auto itFunc = eventsMap.find(index);
        if (itFunc == eventsMap.end())
            return;
        eventsMap.erase(itFunc);
    }

    /// Calls the first subscriber and returns the result
    template <typename Signature, typename... ArgTypes>
    auto CallOne(event_t id, ArgTypes&& ... Arguments) -> typename std::invoke_result<Signature, ArgTypes...>::type
    {
        using ResultType = typename std::invoke_result<Signature, ArgTypes...>::type;
        static constexpr auto isVoid = std::is_same_v<ResultType, void>;

        auto& events = GetEventsT<Signature>();
        const auto it = events.events_.find(id);
        if (it == events.events_.end() || (*it).second.size() == 0)
        {
            // Index not found, return nothing (if void), some default value
            if constexpr(isVoid)
                return;
            else
                // Should work with primitives and classes with trivial constructor
                return ResultType{};
            // or even better throw an exception
        }
        return (*(*it).second.begin()).second(std::forward<ArgTypes>(Arguments)...);
    }

    /// Calls all subscribers and returns a std::vector of results or void
    template <typename Signature, typename... ArgTypes>
    auto CallAll(event_t id, ArgTypes&& ... Arguments)
    {
        using ResultType = typename std::invoke_result<Signature, ArgTypes...>::type;
        static constexpr auto isVoid = std::is_same_v<ResultType, void>;
        auto& events = GetEventsT<Signature>();
        const auto it = events.events_.find(id);
        if (it == events.events_.end() || (*it).second.size() == 0)
        {
            // Index not found, return nothing (if void), some default value
            if constexpr(isVoid)
                return;
            else
                // Return an empty std::vector
                return std::vector<ResultType>();
            // or even better throw an exception
        }

        if constexpr(isVoid)
        {
            for (const auto& fun : (*it).second)
                fun.second(std::forward<ArgTypes>(Arguments)...);
        }
        else
        {
            std::vector<ResultType> result;
            result.reserve((*it).second.size());
            for (const auto& fun : (*it).second)
                result.push_back(fun.second(std::forward<ArgTypes>(Arguments)...));
            return result;
        }
    }

#ifdef SA_EVENTS_ASYNC
    /// Async calls the first subscriber and returns the result
    template <typename Signature, typename... ArgTypes>
    auto AsyncCallOne(event_t id, ArgTypes&& ... Arguments)
    {
        using ResultType = typename std::invoke_result<Signature, ArgTypes...>::type;
        static constexpr auto isVoid = std::is_same_v<ResultType, void>;

        auto& events = GetEventsT<Signature>();
        const auto it = events.events_.find(id);
        if (it == events.events_.end() || (*it).second.size() == 0)
        {
            // Index not found, return nothing (if void), some default value
            if constexpr(isVoid)
                return;
            else
                // Should work with primitives and classes with trivial constructor
                return std::future<ResultType>{};
            // or even better throw an exception
        }
        return std::async(launch_, (*(*it).second.begin()).second, std::forward<ArgTypes>(Arguments)...);
    }

    /// Async calls all subscribers and returns a std::vector of results or void
    template <typename Signature, typename... ArgTypes>
    auto AsyncCallAll(event_t id, ArgTypes&& ... Arguments)
    {
        using ResultType = typename std::invoke_result<Signature, ArgTypes...>::type;
        static constexpr auto isVoid = std::is_same_v<ResultType, void>;
        auto& events = GetEventsT<Signature>();
        const auto it = events.events_.find(id);
        if (it == events.events_.end() || (*it).second.size() == 0)
        {
            // Index not found, return nothing (if void), some default value
            if constexpr(isVoid)
                return;
            else
                // Return an empty std::vector
                return std::vector<std::future<ResultType>>();
            // or even better throw an exception
        }

        if constexpr(isVoid)
        {
            for (const auto& fun : (*it).second)
                std::async(launch_, fun.second, std::forward<ArgTypes>(Arguments)...);
        }
        else
        {
            std::vector<std::future<ResultType>> result;
            result.reserve((*it).second.size());
            for (const auto& fun : (*it).second)
                result.push_back(std::async(launch_, fun.second, std::forward<ArgTypes>(Arguments)...));
            return result;
        }
    }
    std::launch launch_{ std::launch::deferred };
#endif

};

}
