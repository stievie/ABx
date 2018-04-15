#pragma once

namespace Utils {

/// Compile time string hash
/// https://github.com/elanthis/constexpr-hash-demo/blob/master/test.cpp

// FNV-1a constants
#if defined(_WIN64)
static const size_t OFFSET = 14695981039346656037ULL;
static const size_t PRIME = 1099511628211ULL;
#else
static const size_t OFFSET = 2166136261U;
static const size_t PRIME = 16777619U;
#endif

// compile-time hash helper function
constexpr size_t StringHashOne(char c, const char* remain, size_t value)
{
    return c == 0 ? value : StringHashOne(remain[0], remain + 1, (value ^ c) * PRIME);
}

// compile-time hash
constexpr size_t StringHash(const char* str)
{
    return StringHashOne(str[0], str + 1, OFFSET);
}

// run-time hash
inline size_t StringHashRt(const char* str)
{
    size_t hash = OFFSET;
    while (*str != 0) {
        hash ^= str[0];
        hash *= PRIME;
        ++str;
    }
    return hash;
}

}