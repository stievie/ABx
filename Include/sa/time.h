/**
 * Copyright 2020 Stefan Ascher
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

#include <chrono>
#include <iomanip>
#include <time.h>
#include <sa/Compiler.h>

namespace sa {
namespace time {

inline std::tm localtime(const std::time_t& time)
{
    std::tm tm_snapshot;
#if defined(SA_PLATFORM_WIN)
    localtime_s(&tm_snapshot, &time);
#else
    localtime_r(&time, &tm_snapshot); // POSIX
#endif
    return tm_snapshot;
}

inline std::tm gmtime(const std::time_t& time)
{
    std::tm tm_snapshot;
#if defined(SA_PLATFORM_WIN)
    gmtime_s(&tm_snapshot, &time);
#else
    gmtime_r(&time, &tm_snapshot);
#endif
    return tm_snapshot;
}

// To simplify things the return value is just a string. I.e. by design!
inline std::string put_time(const std::tm* date_time, const char* c_time_format)
{
#if defined(SA_PLATFORM_WIN)
    std::ostringstream oss;
    // BOGUS hack done for VS2012: C++11 non-conformant since it SHOULD take a "const struct tm*  "
    // ref. C++11 standard: ISO/IEC 14882:2011, § 27.7.1,
    oss << std::put_time(const_cast<std::tm*>(date_time), c_time_format);
    return oss.str();
#else    // LINUX
    const size_t size = 1024;
    char buffer[size];
    auto success = std::strftime(buffer, size, c_time_format, date_time);
    if (success == 0)
        return c_time_format;
    return buffer;
#endif
}

// extracting std::time_t from std:chrono for "now"
inline std::time_t systemtime_now()
{
    auto system_now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(system_now);
}

struct time_span
{
    uint32_t months{ 0 };
    uint32_t days{ 0 };
    uint32_t hours{ 0 };
    uint32_t minutes{ 0 };
    uint32_t seconds{ 0 };
};

inline time_span get_time_span(uint32_t sec)
{
    time_span result;
    const time_t secs(sec);
    const std::tm p = gmtime(secs);
    if (p.tm_yday > 31)
    {
        result.months = p.tm_yday / 31;
        result.days = p.tm_yday - (result.months * 31);
    }
    else
        result.days = p.tm_yday;
    result.hours = p.tm_hour;
    result.minutes = p.tm_min;
    result.seconds = p.tm_sec;
    return result;
}

inline int64_t tick()
{
    using namespace std::chrono;
    const milliseconds ms = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    );
    return ms.count();
}

/// Return the time that's elapsed since in ms
inline uint32_t time_elapsed(int64_t since)
{
    auto t = tick();
    if (t > since)
        return static_cast<uint32_t>(t - since);
    return 0u;
}

inline bool is_expired(int64_t expiresAt)
{
    return expiresAt < tick();
}

inline std::string format_tick(int64_t tick)
{
    const time_t tm = tick / 1000;
    const std::tm t1 = localtime(tm);
    return put_time(&t1, "%d %b %Y %H:%M");
}

class timer
{
private:
    int64_t start_;
public:
    timer() :
        start_(tick())
    { }
    float elapsed_seconds() const { return static_cast<float>(tick() - start_) / 1000.0f; }
    int64_t elapsed_millis() const { return (tick() - start_); }
    void restart() { start_ = tick(); }
};

}
}
