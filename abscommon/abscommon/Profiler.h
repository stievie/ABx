#pragma once

#include "Logger.h"
#include <chrono>

namespace Utils {

class Profiler
{
private:
    using timer = std::chrono::high_resolution_clock;
    std::chrono::time_point<std::chrono::steady_clock> start_;
    std::string name_;
public:
    Profiler() :
        start_(timer::now())
    {
    }
    Profiler(const std::string name) :
        name_(name),
        start_(timer::now())
    {
    }
    ~Profiler()
    {
#if defined(_PROFILING)
        std::chrono::time_point<std::chrono::steady_clock> end = timer::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
        if (!name_.empty())
            LOG_PROFILE << name_ << ": ";
        if (duration < 1000)
            LOG_PROFILE << duration << "ns" << std::endl;
        else if (duration < 1000 * 1000)
            LOG_PROFILE << duration / 1000 << "us" << std::endl;
        else if (duration < 1000 * 1000 * 1000)
            LOG_PROFILE << duration / (1000 * 1000) << "ms" << std::endl;
        else
            LOG_PROFILE << duration / (1000 * 1000 * 1000) << "s" << std::endl;
#endif
    }
};

}

#if defined(_PROFILING)
#define AB_PROFILE Utils::Profiler UNIQUENAME(__profiler__)(__AB_PRETTY_FUNCTION__)
#else
#define AB_PROFILE
#endif
