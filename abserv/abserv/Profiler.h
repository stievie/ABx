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
        std::chrono::time_point<std::chrono::steady_clock> end = timer::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        if (!name_.empty())
            LOG_INFO << name_ << ": ";
        if (duration < 1000)
            LOG_INFO << duration << "us" << std::endl;
        else if (duration < 1000 * 1000)
            LOG_INFO << duration / 1000 << "ms" << std::endl;
        else
            LOG_INFO << duration / (1000 * 1000) << "s" << std::endl;
    }
};

#if defined(_PROFILING)
#define AB_PROFILE(name) Utils::Profiler __profiler__(name)
#else
#define AB_PROFILE
#endif

}