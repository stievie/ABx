#pragma once

#include <stdint.h>
#include <stdint.h>
#include <sys/timeb.h>
#include <string>
#include <vector>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);
std::string ConvertIPToString(uint32_t ip);
std::vector<std::string> Split(const std::string& str, const std::string& delim);
std::string Trim(const std::string& str, const std::string& whitespace = " \t");

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

}
