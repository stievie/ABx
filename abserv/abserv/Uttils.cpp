#include "stdafx.h"
#include "Utils.h"
#include "Logger.h"

#include "DebugNew.h"

namespace Utils {

std::string Trim(const std::string& str,
    const std::string& whitespace /* = " \t" */)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

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

uint32_t AdlerChecksum(uint8_t* data, int32_t len)
{
    if (len < 0) {
#ifdef _DEBUG
        LOG_ERROR << "len < 0" << std::endl;
#endif
        return 0;
    }
    static const uint32_t MOD_ADLER = 65521;

    uint32_t a = 1, b = 0;
    while (len > 0)
    {
        size_t tlen = len > 5552 ? 5552 : len;
        len -= (int32_t)tlen;
        do
        {
            a += *data++;
            b += a;
        } while (--tlen);

        a %= MOD_ADLER;
        b %= MOD_ADLER;
    }

    return (b << 16) | a;
}

std::string ConvertIPToString(uint32_t ip)
{
    char buffer[20];
    sprintf_s(buffer, 20, "%d.%d.%d.%d", ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24));
    return buffer;
}

}
