#pragma once

#include <stdint.h>
#include <sys/timeb.h>

inline int64_t SysTime()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}
