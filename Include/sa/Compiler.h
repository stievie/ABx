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

#if defined(__GNUC__)
#   define SA_GCC
#elif defined(__clang__)
#   define SA_CLANG
#elif defined(_MSC_VER)
#   define SA_MSVC
#endif

#if defined(__ARM_ARCH) || defined(_M_ARM) || defined(_M_ARM64)
#   define SA_ARCH_ARM
#elif defined(SA_MSVC)
#   if defined(_M_X86) || defined(_M_X64)
#       define SA_ARCH_X86
#   endif
#elif defined(SA_GCC) || defined(SA_CLANG)
#   if defined(__i386__) || defined(__x86_64__)
#       define SA_ARCH_X86
#   endif
#endif

#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
#   define SA_ARCH_64BIT
#else
#   define SA_ARCH_32BIT
#endif

#if defined(_WIN32)
#   define SA_PLATFORM_WIN
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if defined(TARGET_OS_IPHONE)
#       define SA_PLATFORM_IOS
#   else
#       define SAPLATFORM_OSX
#   endif
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#   define SA_PLATFORM_LINUX
#elif defined(__unix__)
#   define SA_PLATFORM_UNIX
#elif defined(__ANDROID__)
#   define SA_PLATFORM_ANDROID
#endif

#define SA_COMPILER_STRINGIFY(a) #a

#if defined(SA_MSVC)
#   define PRAGMA_WARNING_PUSH __pragma(warning(push))
#   define PRAGMA_WARNING_POP __pragma(warning(pop))
#   define PRAGMA_WARNING_DISABLE_MSVC(id) __pragma(warning(disable: id))
#   define PRAGMA_WARNING_DISABLE_GCC(id)
#   define PRAGMA_WARNING_DISABLE_CLANG(id)
#elif defined(SA_GCC)
#   define PRAGMA_WARNING_PUSH _Pragma("GCC diagnostic push")
#   define PRAGMA_WARNING_POP _Pragma("GCC diagnostic pop")
#   define PRAGMA_WARNING_DISABLE_MSVC(id)
#   define PRAGMA_WARNING_DISABLE_GCC(id) _Pragma(SA_COMPILER_STRINGIFY(GCC diagnostic ignored id))
#   define PRAGMA_WARNING_DISABLE_CLANG(id)
#elif defined(SA_CLANG)
#   define PRAGMA_WARNING_PUSH _Pragma("clang diagnostic push")
#   define PRAGMA_WARNING_POP _Pragma("clang diagnostic pop")
#   define PRAGMA_WARNING_DISABLE_MSVC(id)
#   define PRAGMA_WARNING_DISABLE_GCC(id)
#   define PRAGMA_WARNING_DISABLE_CLANG(id) _Pragma(SA_COMPILER_STRINGIFY(clang diagnostic ignored id))
#endif

#if defined(SA_GCC)
#   define SA_ALWAYS_INLINE [[gnu::always_inline]] inline
#elif defined(SA_CLANG)
#   define SA_ALWAYS_INLINE inline __attribute__((always_inline))
#elif defined(SA_MSVC)
#   define SA_ALWAYS_INLINE __forceinline
#endif

#if defined(SA_GCC) || defined(SA_CLANG)
// GCC's __FUNCTION__ does not resolve namespace
#   define SA_FUNCTION __PRETTY_FUNCTION__
#elif defined(SA_MSVC)
#   define SA_FUNCTION __FUNCTION__
#endif

// Function with signature
#if defined(SA_GCC) || defined(SA_CLANG)
#   define SA_FUNCTION_SIG __PRETTY_FUNCTION__
#elif defined(SA_MSVC)
#   define SA_FUNCTION_SIG __FUNCSIG__
#endif
