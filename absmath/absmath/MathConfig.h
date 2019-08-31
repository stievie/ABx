#pragma once

// DirectXMath also works on Windows on ARM and it should also compile on Linux now.

#if defined(__ARM_ARCH) || defined(_M_ARM) || defined(_M_ARM64)
#   define BUILD_ARCH_ARM 1
#elif defined(_MSC_VER)
#   if defined(_M_X86) || defined(_M_X64)
#       define BUILD_ARCH_X86 1
#   endif
#elif defined(__GNUC__) || defined(__clang__)
#   if defined(__i386__) || defined(__x86_64__)
#       define BUILD_ARCH_X86 1
#   endif
#endif

#if defined(_WIN32)
#   define BUILD_PLATFORM_WIN 1
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if defined(TARGET_OS_IPHONE)
#       define BUILD_PLATFORM_IOS 1
#   else
#       define BUILD_PLATFORM_OSX 1
#   endif
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#   define BUILD_PLATFORM_LINUX 1
#elif defined(__unix__)
#   define BUILD_PLATFORM_UNIX 1
#elif defined(__ANDROID__)
#   define BUILD_PLATFORM_ANDROID 1
#endif

#define HAVE_DIRECTX_MATH
/*
 * BUILD_INTRINSICS_LEVEL 0..3. Try setting different levels and see what compiles.
 * 0 _XM_NO_INTRINSICS_
 * 1 Some
 * 2 More
 * 3 All
 *
 * If not defined it tries to auto detect it
 */

// If BUILD_INTRINSICS_LEVEL not defined try to auto detect
#if !defined (BUILD_INTRINSICS_LEVEL)
#   ifdef _MSC_VER
#       if _MSC_VER >= 1300
#           define _XM_SSE_INTRINSICS_
#       endif
#       if _MSC_VER >= 1500
#           define _XM_SSE3_INTRINSICS_
#           define _XM_SSE4_INTRINSICS_
#       endif
#       if _MSC_FULL_VER >= 160040219
#           define _XM_AVX_INTRINSICS_
#       endif
#       if _MSC_VER >= 1700
//#           define _XM_AVX2_INTRINSICS_
#           define _XM_F16C_INTRINSICS_
#       endif
#   else
#       if defined(__SSE__)
#           define _XM_SSE_INTRINSICS_
#       endif
#       if defined(__SSE3__)
#           define _XM_SSE3_INTRINSICS_
#       endif
#       if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#           define _XM_SSE4_INTRINSICS_
#       endif
#       if defined(__AVX__)
#           define _XM_AVX_INTRINSICS_
#       endif
#       if defined(__AVX2__)
#           define _XM_AVX2_INTRINSICS_
#       endif
#       if defined(__F16C__)
#           define _XM_F16C_INTRINSICS_
#       endif
#   endif
#endif

#if defined(BUILD_INTRINSICS_LEVEL)
#   if defined(BUILD_ARCH_ARM)
#       if defined(__ARM_NEON) && BUILD_INTRINSICS_LEVEL > 0
#           define _XM_ARM_NEON_INTRINSICS_
#       else
#           define _XM_NO_INTRINSICS_
#       endif
#   else
#       if BUILD_INTRINSICS_LEVEL == 0
#           define _XM_NO_INTRINSICS_
#       endif
#       if BUILD_INTRINSICS_LEVEL > 0
#           define _XM_SSE_INTRINSICS_
#       endif
#       if BUILD_INTRINSICS_LEVEL > 1
#           define _XM_SSE3_INTRINSICS_
#           define _XM_SSE4_INTRINSICS_
#           define _XM_AVX_INTRINSICS_
//#           define _XM_AVX2_INTRINSICS_  // <-- Crashes in directxcollision.inl: line: 1191
#       endif
#       if BUILD_INTRINSICS_LEVEL > 2
#           define _XM_F16C_INTRINSICS_
#       endif
#   endif
#endif

#if defined(__GNUC__) || defined(BUILD_PLATFORM_IOS)
#   define _XM_NO_CALL_CONVENTION_
#endif
#if defined(BUILD_PLATFORM_IOS) || defined(BUILD_PLATFORM_ANDROID)
#   define _XM_ARM_NEON_NO_ALIGN_
#endif

#if defined(HAVE_DIRECTX_MATH)
#   if defined(__clang__)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wpadded"
#       pragma clang diagnostic ignored "-Wunused-but-set-variable"
#   endif
#   if defined(__GNUC__)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wpadded"
#       pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#   endif

#   include <DirectXMath.h>
#   include <DirectXCollision.h>

#   if defined(__GNUC__)
#       pragma GCC diagnostic pop
#   endif
#   if defined(__clang__)
#       pragma clang diagnostic pop
#   endif
namespace XMath = DirectX;
#endif
