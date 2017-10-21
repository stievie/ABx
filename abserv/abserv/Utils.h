#pragma once

#include <stdint.h>
#include <stdint.h>
#include <sys/timeb.h>
#include <string>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);
std::string ConvertIPToString(uint32_t ip);

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

}
