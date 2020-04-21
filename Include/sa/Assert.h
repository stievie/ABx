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

// Provide assert() even when NDEBUG is defined, i.e. release builds. In Debug build nothing changes.
// SA_ASSERT must be defined.

#include <cassert>
#include <stdlib.h>
#include <iostream>

#ifdef SA_ASSERT

#ifdef NDEBUG

#if defined(__GNUC__)
#   define SA_ASSERT_INLINE [[gnu::always_inline]] inline
#elif defined(__clang__)
#   define SA_ASSERT_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#   define SA_ASSERT_INLINE __forceinline
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define SA_ASSERT_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#   define SA_ASSERT_FUNCTION __FUNCTION__
#endif

[[noreturn]] SA_ASSERT_INLINE void sa_assertion_failed(const char* msg, const char* file, unsigned line, const char* func)
{
    std::cerr << "Assertion failed: " << msg << " in " << file << ":" << line << " " << func << std::endl;
    abort();
}

#if defined(assert)
#undef assert
#endif
#define assert(expr) (static_cast<bool>(expr) ? (void)0 : sa_assertion_failed(#expr, __FILE__, __LINE__, SA_ASSERT_FUNCTION))

#endif

#endif

// Some convenience macros

#define ASSERT_FALSE() assert(false)
