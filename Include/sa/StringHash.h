/**
 * Copyright 2017-2020 Stefan Ascher
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

#include <stdlib.h>
#if __cplusplus >= 201703L
#include <string_view>
#endif

namespace sa {

/// Compile time string hash
/// https://github.com/elanthis/constexpr-hash-demo/blob/master/test.cpp

// FNV-1a constants
#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
static_assert(sizeof(size_t) == 8);
static const size_t OFFSET = 14695981039346656037ULL;
static const size_t PRIME = 1099511628211ULL;
#else
static_assert(sizeof(size_t) == 4);
static const size_t OFFSET = 2166136261U;
static const size_t PRIME = 16777619U;
#endif

/// Compile-time hash helper function
constexpr size_t StringHashOne(char c, const char* remain, size_t value)
{
    return c == 0 ? value : StringHashOne(remain[0], remain + 1, (value ^ static_cast<size_t>(c)) * PRIME);
}

/// Compile-time hash
constexpr size_t StringHash(const char* str)
{
    return StringHashOne(str[0], str + 1, OFFSET);
}

// MSVC requires /Zc:__cplusplus option, see https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
#if __cplusplus >= 201703L
/// Compile time string hash of string_view. Requires C++17.
constexpr size_t StringHash(std::string_view str)
{
    size_t hash = OFFSET;
    for (auto c : str)
    {
        hash ^= static_cast<size_t>(c);
        hash *= PRIME;
    }
    return hash;
}
#endif

namespace literals {
constexpr size_t operator"" _Hash(const char* str, size_t) { return StringHash(str); }
}

/// Run-time hash
inline size_t StringHashRt(const char* str)
{
    size_t hash = OFFSET;
    while (*str != 0) {
        hash ^= static_cast<size_t>(str[0]);
        hash *= PRIME;
        ++str;
    }
    return hash;
}

/// Run time string hash, supports \0 inside
inline size_t StringHashRt(const char* str, size_t size)
{
    size_t hash = OFFSET;
    for (size_t i = 0; i < size; ++i)
    {
        hash ^= static_cast<size_t>(str[i]);
        hash *= PRIME;
    }
    return hash;
}

}
