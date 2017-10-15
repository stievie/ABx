#pragma once

#include <stdint.h>
#include <sys/timeb.h>
#include <string>

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}
