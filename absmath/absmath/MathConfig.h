#pragma once

#if defined(_WIN32)
// DirectXMath also works on Windows on ARM
#   define HAVE_DIRECTX_MATH
#   if !defined(BUILD_INTRINSICS_LEVEL)
#       define BUILD_INTRINSICS_LEVEL 3
#   endif
#else
#   define HAVE_X_MATH
#   if !defined(BUILD_INTRINSICS_LEVEL)
#       define BUILD_INTRINSICS_LEVEL 1
#   endif
#endif

#if !defined(HAVE_DIRECTX_MATH) && !defined(HAVE_X_MATH)
#error HAVE_DIRECTX_MATH or HAVE_X_MATH must be defined
#endif

#if defined(BUILD_ARCH_ARM)
#   if defined(__ARM_NEON) && BUILD_INTRINSICS_LEVEL > 0
#       define _XM_ARM_NEON_INTRINSICS_
#   else
#       define _XM_NO_INTRINSICS_
#   endif
#else
#   if BUILD_INTRINSICS_LEVEL > 0
#       define _XM_SSE_INTRINSICS_
#   endif
#   if BUILD_INTRINSICS_LEVEL > 1
#       define _XM_SSE3_INTRINSICS_
#       define _XM_SSE4_INTRINSICS_
#       define _XM_AVX_INTRINSICS_
#   endif
#   if BUILD_INTRINSICS_LEVEL > 2
#       define _XM_F16C_INTRINSICS_
#   endif
#endif
#if defined(VE_COMPILER_GCC) || defined(BUILD_PLATFORM_IOS)
#   define _XM_NO_CALL_CONVENTION_
#endif
#if defined(BUILD_PLATFORM_IOS) || defined(BUILD_PLATFORM_ANDROID)
#   define _XM_ARM_NEON_NO_ALIGN_
#endif
//#define _XM_NO_INTRINSICS_

#if defined(HAVE_DIRECTX_MATH)
#   include <DirectXMath.h>
#   include <DirectXCollision.h>
namespace XMath = DirectX;
#elif defined(HAVE_X_MATH)
#   include <XMath.h>
#   include <XCollision.h>
#endif
