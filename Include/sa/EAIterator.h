#pragma once

#include <EASTL/iterator.h>

namespace sa {
namespace ea {

template<typename Iter, typename RandomGenerator>
Iter SelectRandomly(Iter start, Iter end, RandomGenerator& g)
{
    std::uniform_int_distribution<> dis(0, static_cast<int>(::eastl::distance(start, end)) - 1);
    ::eastl::advance(start, dis(g));
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
    int adv = static_cast<int>(round(static_cast<float>(::eastl::distance(start, end) - 1) * rnd));
    ::eastl::advance(start, adv);
    return start;
}

}
}