#pragma once

#include <stdlib.h>
#include <string_view>

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
