#pragma once

#include <stdint.h>
#include <stdint.h>
#include <sys/timeb.h>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

}
