#pragma once

#include <thread>
#include <chrono>

namespace sa {

/// This thread sleeps until pred returns true
template<typename Predicate>
void ConditionSleep(Predicate&& pred, unsigned maxWait = 0)
{
    using namespace std::chrono_literals;
    unsigned waited = 0;
    while (!pred())
    {
        std::this_thread::sleep_for(100ms);
        waited += 100;
        if (maxWait != 0 && waited >= maxWait)
            break;
    }
}

}
