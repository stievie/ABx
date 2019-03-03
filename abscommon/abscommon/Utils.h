#pragma once

#include <sys/timeb.h>
#include <time.h>
#include <random>
#include <iterator>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

inline uint32_t TimePassed(int64_t last)
{
    return static_cast<uint32_t>(Utils::AbTick() - last);
}

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g)
{
    std::uniform_int_distribution<> dis(0, static_cast<int>(std::distance(start, end)) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

bool GetCommandLineValue(const std::vector<std::string>& values, const std::string& name);
bool GetCommandLineValue(const std::vector<std::string>& values, const std::string& name, std::string& value);

}
