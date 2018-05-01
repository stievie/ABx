#pragma once

#include <stdint.h>
#include <sys/timeb.h>
#include <time.h>
#include <string>

namespace Client {

typedef int64_t ticks_t;

ticks_t time();
ticks_t millis();
ticks_t micros();
void millisleep(size_t ms);
void microsleep(size_t us);
std::string format_tick(int64_t tick);

struct timer
{
private:
    ticks_t start_;
public:
    timer() { restart(); }
    float elapsed_seconds() { return (float)((micros() - start_) / 1000000.0); }
    ticks_t elapsed_millis() { return (micros() - start_) / 1000; }
    ticks_t elapsed_micros() { return micros() - start_; }
    void restart() { start_ = micros(); }
};

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

struct TimeSpan
{
    uint32_t months = 0;
    uint32_t days = 0;
    uint32_t hours = 0;
    uint32_t minutes = 0;
    uint32_t seconds = 0;
    TimeSpan(uint32_t sec)
    {
        time_t secs(sec); // you have to convert your input_seconds into time_t
        struct tm p;
        errno_t err = gmtime_s(&p, &secs); // convert to broken down time
        if (err)
            return;

        if (p.tm_yday > 31)
        {
            months = p.tm_yday / 31;
            days = p.tm_yday - (months * 31);
        }
        else
            days = p.tm_yday;
        hours = p.tm_hour;
        minutes = p.tm_min;
        seconds = p.tm_sec;
    }
};

}
