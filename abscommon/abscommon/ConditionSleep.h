#pragma once

#include <thread>
#include <chrono>

namespace Asynch {

/// This thread sleeps until pred is met
template<typename Predicate>
void ConditionSleep(Predicate&& pred)
{
    using namespace std::chrono_literals;
    while (!pred())
        std::this_thread::sleep_for(100ms);
}

}
