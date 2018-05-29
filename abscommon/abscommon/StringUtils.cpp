#include "stdafx.h"
#include "StringUtils.h"
#include <sstream>

namespace Utils {

std::vector<std::string> Split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> parts;
    char* input = (char*)str.c_str();
    const char* d = delim.c_str();
    char* next;
    char* pos = strtok_s(input, d, &next);
    while (pos != NULL)
    {
        parts.push_back(std::string(pos));
        pos = strtok_s(NULL, d, &next);
    }
    return parts;
}

bool IsNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string ConvertIPToString(uint32_t ip, bool mask /* = false */)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    char buffer[INET_ADDRSTRLEN];
    if (!mask)
        sprintf_s(buffer, INET_ADDRSTRLEN, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]);
    else
        sprintf_s(buffer, INET_ADDRSTRLEN, "%u.%u.x.x", bytes[3], bytes[2]);
    return buffer;
}

uint32_t ConvertStringToIP(const std::string& ip)
{
    unsigned int byte3;
    unsigned int byte2;
    unsigned int byte1;
    unsigned int byte0;

    if (sscanf_s(ip.c_str(), "%u.%u.%u.%u", &byte3, &byte2, &byte1, &byte0) == 4)
    {
        if ((byte3 < 256)
            && (byte2 < 256)
            && (byte1 < 256)
            && (byte0 < 256))
        {
            return (byte3 << 24)
                + (byte2 << 16)
                + (byte1 << 8)
                + byte0;
        }
    }

    return 0;
}

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
