#pragma once

#include <utility>

namespace sa {

template <typename ForwardIterator, typename OutputIterator, typename UnaryPredicate>
inline void SelectIterators(ForwardIterator first, ForwardIterator last,
    OutputIterator out, UnaryPredicate pred)
{
    while (first != last)
    {
        if (pred(*first))
            *out++ = first;
        ++first;
    }
}

template<typename Iter, typename RandomGenerator>
Iter SelectRandomly(Iter start, Iter end, RandomGenerator& g)
{
    std::uniform_int_distribution<> dis(0, static_cast<int>(std::distance(start, end)) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter SelectRandomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return SelectRandomly(start, end, gen);
}

template<typename Iter>
Iter SelectRandomly(Iter start, Iter end, float rnd)
{
    int adv = static_cast<int>(round(static_cast<float>(std::distance(start, end) - 1) * rnd));
    std::advance(start, adv);
    return start;
}

}
