#pragma once

#include <sys/timeb.h>
#include <time.h>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

}
