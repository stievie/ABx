#include "stdafx.h"
#include "StringUtils.h"
#include <sstream>

namespace Utils {

static float RoundOff(float n)
{
    float d = n * 100.0f;
    int i = static_cast<int>(d + 0.5);
    d = (float)i / 100.0f;
    return d;
}

std::string ConvertSize(size_t size)
{
    static const char *SIZES[] = { "B", "KB", "MB", "GB" };
    int div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES))
    {
        rem = (size % 1024);
        div++;
        size /= 1024;
    }

    float size_d = (float)size + (float)rem / 1024.0f;
    std::ostringstream convert;
    convert << RoundOff(size_d);
    std::string result = convert.str() + " " + SIZES[div];
    return result;
}

}
