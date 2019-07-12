#include "stdafx.h"
#include "TimeUtils.h"
#include <chrono>
#include <ctime>
#include <thread>

namespace Client {

const static auto gStartupTtime = std::chrono::high_resolution_clock::now();

ticks_t time()
{
    return std::time(nullptr);
}

ticks_t millis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - gStartupTtime).count();
}

ticks_t micros()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - gStartupTtime).count();
}

void millisleep(size_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void microsleep(size_t us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

std::string format_tick(int64_t tick)
{
    time_t tm = tick / 1000;
    char buff[256];
#ifdef _MSC_VER
    struct tm t1;
    if (localtime_s(&t1, &tm))
        return std::string();
    size_t size = strftime(buff, 256, "%d %b %Y %H:%M", &t1);
#else
    struct tm* t1;
    t1 = localtime (&tm);
    if (!t1)
        return std::string();
    size_t size = strftime(buff, 256, "%d %b %Y %H:%M", t1);
#endif
    return std::string(buff, size);
}

}
