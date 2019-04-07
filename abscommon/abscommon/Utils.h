#pragma once

#include <sys/timeb.h>
#include <time.h>
#include <random>
#include <iterator>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);

template <typename T, int N>
constexpr size_t CountOf(T(&)[N])
{
    return N;
}

inline int64_t Tick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

inline uint32_t TimePassed(int64_t last)
{
    auto tick = Tick();
    if (tick > last)
        return static_cast<uint32_t>(tick - last);
    return 0u;
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
    int adv = static_cast<int>(static_cast<float>(std::distance(start, end) - 1) * rnd);
    std::advance(start, adv);
    return start;
}

bool GetCommandLineValue(const std::vector<std::string>& values, const std::string& name);
bool GetCommandLineValue(const std::vector<std::string>& values, const std::string& name, std::string& value);

}
