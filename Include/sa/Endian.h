/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <sa/Compiler.h>
#include <stdint.h>

#ifdef __has_include
#   if __has_include(<endian.h>)
#       include <endian.h>
#   elif __has_include(<machine/endian.h>)
#       include <machine/endian.h>
#   elif __has_include(<sys/param.h>)
#       include <sys/param.h>
#   elif __has_include(<sys/isadefs.h>)
#       include <sys/isadefs.h>
#   endif
#endif

#if defined(__LITTLE_ENDIAN__)
#   define SA_LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN__)
#   define SA_BIG_ENDIAN
#else
#   if (defined(__BYTE_ORDER__)  && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || \
        (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || \
        (defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN) || \
        (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) || \
        (defined(__sun) && defined(__SVR4) && defined(_BIG_ENDIAN)) || \
        defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
        defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__) || \
        defined(_M_PPC)
#   define SA_BIG_ENDIAN
#   elif (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || /* gcc */\
        (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) /* linux header */ || \
        (defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN) || \
        (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) /* mingw header */ ||  \
        (defined(__sun) && defined(__SVR4) && defined(_LITTLE_ENDIAN)) || /* solaris */ \
        defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || \
        defined(_M_IX86) || defined(_M_X64) || defined(_M_IA64) || /* msvc for intel processors */ \
        defined(_M_ARM) /* msvc code on arm executes in little endian mode */
#       define SA_LITTLE_ENDIAN
#   endif
#endif

namespace sa {

#if defined(_MSC_VER)
#   define sa_bswap16(x)     _byteswap_ushort((x))
#   define sa_bswap32(x)     _byteswap_ulong((x))
#   define sa_bswap64(x)     _byteswap_uint64((x))
#elif (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#   define sa_bswap16(x)     __builtin_sa_bswap16((x))
#   define sa_bswap32(x)     __builtin_sa_bswap32((x))
#   define sa_bswap64(x)     __builtin_sa_bswap64((x))
#elif defined(__has_builtin) && __has_builtin(__builtin_sa_bswap64)
#   define sa_bswap16(x)     __builtin_sa_bswap16((x))
#   define sa_bswap32(x)     __builtin_sa_bswap32((x))
#   define sa_bswap64(x)     __builtin_sa_bswap64((x))
#else
static constexpr uint16_t sa_bswap16(uint16_t x)
{
    return (((x >> 8) & 0xffu) | ((x & 0xffu) << 8));
}
static constexpr uint32_t sa_bswap32(uint32_t x)
{
    return (((x & 0xff000000u ) >> 24) |
            ((x & 0x00ff0000u ) >> 8 ) |
            ((x & 0x0000ff00u ) << 8 ) |
            ((x & 0x000000ffu ) << 24));
}
static constexpr uint64_t sa_bswap64(uint64_t x)
{
    return (((x & 0xff00000000000000ull) >> 56) |
            ((x & 0x00ff000000000000ull) >> 40) |
            ((x & 0x0000ff0000000000ull) >> 24) |
            ((x & 0x000000ff00000000ull) >> 8 ) |
            ((x & 0x00000000ff000000ull) << 8 ) |
            ((x & 0x0000000000ff0000ull) << 24) |
            ((x & 0x000000000000ff00ull) << 40) |
            ((x & 0x00000000000000ffull) << 56));
}
#endif

template<typename T>
constexpr T ConvertLittleEndian(T value)
{
#if defined(SA_LITTLE_ENDIAN)
    return value;
#elif defined(SA_BIG_ENDIAN)
    if constexpr (sizeof(T) == 8)
        return sa_bswap64(value);
    if constexpr (sizeof(T) == 4)
        return sa_bswap32(value);
    if constexpr (sizeof(T) == 2)
        return sa_bswap16(value);
    if constexpr (sizeof(T) == 1)
        return value;
#endif
}

template<typename T>
constexpr T ConvertBigEndian(T value)
{
#if defined(SA_LITTLE_ENDIAN)
    if constexpr (sizeof(T) == 8)
        return sa_bswap64(value);
    if constexpr (sizeof(T) == 4)
        return sa_bswap32(value);
    if constexpr (sizeof(T) == 2)
        return sa_bswap16(value);
    if constexpr (sizeof(T) == 1)
        return value;
#elif defined(SA_BIG_ENDIAN)
    return value;
#endif
}

template<typename T>
constexpr T ConvertNetworkEndian(T value)
{
    return ConvertBigEndian(value);
}

}
