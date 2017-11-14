#include "stdafx.h"
#include "Utils.h"

uint32_t AdlerChecksum(uint8_t* data, int32_t len)
{
    if (len < 0) {
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
