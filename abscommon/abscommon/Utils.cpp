#include "stdafx.h"
#include "Utils.h"
#include "Logger.h"

namespace Utils {

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

bool GetCommandLineValue(const std::vector<std::string>& values, const std::string & name)
{
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i].compare(name) == 0)
        {
            return true;
        }
    }
    return false;
}

bool GetCommandLineValue(const std::vector<std::string>& values, const std::string& name, std::string& value)
{
    bool found = false;
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i].compare(name) == 0)
        {
            found = true;
            ++i;
            if (i < values.size())
                value = values[i];
            else
            {
                LOG_WARNING << "Missing argument for " << name << std::endl;
            }
            break;
        }
    }
    return found;
}

}
