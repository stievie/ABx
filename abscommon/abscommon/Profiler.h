#pragma once

#include "Logger.h"
#include <chrono>

// Undefine PROFILING or define NPROFILING to disable profiling

// Used by the profiler to generate a unique identifier
#if (!defined(CONCAT))
#define CONCAT(a, b) a ## b
#endif
#if (!defined(UNIQUENAME))
#define UNIQUENAME(prefix) CONCAT(prefix, __LINE__)
#endif

namespace Utils {

class Profiler
{
private:
    using timer = std::chrono::high_resolution_clock;
    std::chrono::time_point<timer> start_;
    std::string name_;
public:
    Profiler() :
        start_(timer::now())
    { }
    Profiler(const std::string& name) :
        start_(timer::now()),
        name_(name)
    { }
    ~Profiler()
    {
#if defined(PROFILING) && !defined(NPROFILING)
        std::chrono::time_point<timer> end = timer::now();
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

#if defined(PROFILING) && !defined(NPROFILING)
#   define AB_PROFILE Utils::Profiler UNIQUENAME(__profiler__)(AB_PRETTY_FUNCTION)
#else
#   define AB_PROFILE
#endif
