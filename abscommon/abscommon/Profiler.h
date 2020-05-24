/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "Logger.h"
#include <chrono>
#include <string_view>

// Undefine PROFILING or define NPROFILING to disable profiling

// Used by the profiler to generate a unique identifier
#if (!defined(CONCAT))
#define CONCAT(a, b) a ## b
#endif
#if (!defined(CONCAT2))
#define CONCAT2(a, b, c) a ## b ## c
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
    std::string_view name_;
    unsigned line_{ 0 };
public:
    Profiler() :
        start_(timer::now())
    { }
    Profiler(std::string_view name, unsigned line) :
        start_(timer::now()),
        name_(name),
        line_(line)
    { }
    ~Profiler()
    {
#if defined(PROFILING) && !defined(NPROFILING)
        std::chrono::time_point<timer> end = timer::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
        if (!name_.empty())
        {
            LOG_PROFILE << name_;
            if (line_ != 0)
                LOG_PROFILE << "#" << line_;
            LOG_PROFILE << " ";
        }
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
#   define AB_PROFILE Utils::Profiler UNIQUENAME(__profiler__)(SA_FUNCTION, __LINE__)
#else
#   define AB_PROFILE
#endif
