#pragma once

#include <stdint.h>
#include <sys/timeb.h>
#include <time.h>

namespace Client {

typedef int64_t ticks_t;

ticks_t time();
ticks_t millis();
ticks_t micros();
void millisleep(size_t ms);
void microsleep(size_t us);

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

}
