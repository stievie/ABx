#include "stdafx.h"
#include "Time.h"
#include <chrono>
#include <ctime>
#include <thread>

namespace Client {

const static auto gStartupTtime = std::chrono::high_resolution_clock::now();

ticks_t time()
{
    return std::time(NULL);
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

}
