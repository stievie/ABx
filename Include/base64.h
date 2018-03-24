// https://github.com/gaspardpetit/base64
// https://raw.githubusercontent.com/gaspardpetit/base64/master/src/polfosol/polfosol.h
// https://github.com/gaspardpetit/base64/blob/master/src/JouniMalinen/jounimalinen.h
// http://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

#pragma once

#include <string>
#include <cstring>

namespace base64 {

static constexpr const unsigned char base64_index[256] = {
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 62  , 0x80, 62  , 0x80, 63  ,
    52  , 53  , 54  , 55  , 56  , 57  , 58  , 59  , 60  , 61  , 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0   , 1   , 2   , 3   , 4   , 5   , 6   , 7   , 8   , 9   , 10  , 11  , 12  , 13  , 14  ,
    15  , 16  , 17  , 18  , 19  , 20  , 21  , 22  , 23  , 24  , 25  , 0x80, 0x80, 0x80, 0x80, 63  ,
    0x80, 26  , 27  , 28  , 29  , 30  , 31  , 32  , 33  , 34  , 35  , 36  , 37  , 38  , 39  , 40  ,
    41  , 42  , 43  , 44  , 45  , 46  , 47  , 48  , 49  , 50  , 51  , 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

static const unsigned int base64_chars[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

std::string encode(const unsigned char* src, size_t len)
#if !defined(BASE64_IMPLEMENTATION)
;
#else
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return std::string(); /* integer overflow */

    std::string outStr;
    outStr.resize(olen);
    out = (unsigned char*)&outStr[0];

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3)
    {
        *pos++ = (unsigned char)base64_chars[in[0] >> 2];
        *pos++ = (unsigned char)base64_chars[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = (unsigned char)base64_chars[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = (unsigned char)base64_chars[in[2] & 0x3f];
        in += 3;
    }

    if (end - in)
    {
        *pos++ = (unsigned char)base64_chars[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = (unsigned char)base64_chars[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = (unsigned char)base64_chars[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = (unsigned char)base64_chars[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}
#endif

std::string decode(const void* data, const size_t len)
#if !defined(BASE64_IMPLEMENTATION)
;
#else
{
    unsigned char* p = (unsigned char*)data;
    int pad = len > 0 && (len % 4 || p[len - 1] == '=');
    const size_t L = ((len + 3) / 4 - pad) * 4;
    std::string str;
    str.resize(3 * ((len + 3) / 4));

    int j = 0;
    for (size_t i = 0; i < L; i += 4)
    {
        int n = base64_index[p[i]] << 18 | base64_index[p[i + 1]] << 12 |
            base64_index[p[i + 2]] << 6 | base64_index[p[i + 3]];
        str[j++] = (char)(n >> 16);
        str[j++] = (char)(n >> 8 & 0xFF);
        str[j++] = (char)(n & 0xFF);
    }
    if (pad)
    {
        int n = base64_index[p[L]] << 18 | base64_index[p[L + 1]] << 12;
        str[j++] = (char)(n >> 16);

        if (len > L + 2 && p[L + 2] != '=')
        {
            n |= base64_index[p[L + 2]] << 6;
            str[j++] = (char)(n >> 8 & 0xFF);
        }
    }

    str.resize(j);
    return std::move(str);
}
#endif

std::string decode(const std::string& str64)
#if !defined(BASE64_IMPLEMENTATION)
;
#else
{
    return decode(str64.data(), str64.length());
}
#endif

}
