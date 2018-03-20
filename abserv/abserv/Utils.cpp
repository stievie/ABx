#include "stdafx.h"
#include "Utils.h"
#include "Logger.h"

#include "DebugNew.h"

namespace Utils {

bool IsNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool FileCopy(const std::string& src, const std::string& dst)
{
    std::ifstream fsrc(src, std::ios::binary);
    if (!fsrc.is_open())
        return false;
    std::ofstream fdst(dst, std::ios::binary);
    if (!fdst.is_open())
        return false;
    fdst << fsrc.rdbuf();
    return true;
}

std::string NormalizeFilename(const std::string& filename)
{
    std::string normal_name(filename);
    ReplaceSubstring<char>(normal_name, "\\", "/");
    while (ReplaceSubstring<char>(normal_name, "//", "/"));
    return normal_name;
}

bool FileExists(const std::string& name)
{
    std::ifstream infile(name.c_str());
    return infile.good();
}

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

std::string LeftTrim(const std::string& str,
    const std::string& whitespace /* = " \t" */)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    return str.substr(strBegin, std::string::npos);
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
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    char buffer[INET_ADDRSTRLEN];
    sprintf_s(buffer, INET_ADDRSTRLEN, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]);
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

}
