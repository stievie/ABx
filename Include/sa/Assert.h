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

// Provide assert() even when NDEBUG is defined, i.e. release builds. In Debug build nothing changes.
// SA_ASSERT must be defined.

#include <cassert>
#include <stdlib.h>
#include <iostream>
#include <sa/Compiler.h>

#ifdef SA_ASSERT

#ifdef NDEBUG

namespace sa {
namespace details {

[[noreturn]] SA_ALWAYS_INLINE void assertion_failed(const char* msg, const char* file, unsigned line, const char* func)
{
    std::cerr << "Assertion failed: " << msg << " in " << file << ":" << line << " " << func << std::endl;
    abort();
}

}
}

#define ASSERT(expr) (static_cast<bool>(expr) ? (void)0 : sa::details::assertion_failed(#expr, __FILE__, __LINE__, SA_FUNCTION_SIG))
#else
#define ASSERT assert
#endif  // NDEBUG
#else
#define ASSERT(expr)
#endif  // SA_ASSERT

// NOTE: ASSERT_FALSE() never returns, no matter what is defined, i.e. also not in release builds.
#if !defined(NDEBUG) || defined(SA_ASSERT)
// There is an abort() to make sure it really does not return, even when the assert() macro doesn't do much.
#define ASSERT_FALSE()           \
    do                           \
    {                            \
        ASSERT(false);           \
        abort();                 \
    } while (0)
#else
#define ASSERT_FALSE() abort()
#endif
