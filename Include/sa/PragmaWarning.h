#pragma once

#define PRAGMA_STRINGIFY(a) #a

#if defined(_MSC_VER)
#   define PRAGMA_WARNING_PUSH __pragma(warning(push))
#   define PRAGMA_WARNING_POP __pragma(warning(pop))
#   define PRAGMA_WARNING_DISABLE_MSVC(id) __pragma(warning(disable: id))
#   define PRAGMA_WARNING_DISABLE_GCC(id)
#   define PRAGMA_WARNING_DISABLE_CLANG(id)
#elif defined(__GNUC__)
#   define PRAGMA_WARNING_PUSH _Pragma("GCC diagnostic push")
#   define PRAGMA_WARNING_POP _Pragma("GCC diagnostic pop")
#   define PRAGMA_WARNING_DISABLE_MSVC(id)
#   define PRAGMA_WARNING_DISABLE_GCC(id) _Pragma(PRAGMA_STRINGIFY(GCC diagnostic ignored id))
#   define PRAGMA_WARNING_DISABLE_CLANG(id)
#elif defined(__clang__)
#   define PRAGMA_WARNING_PUSH _Pragma("clang diagnostic push")
#   define PRAGMA_WARNING_POP _Pragma("clang diagnostic pop")
#   define PRAGMA_WARNING_DISABLE_MSVC(id)
#   define PRAGMA_WARNING_DISABLE_GCC(id)
#   define PRAGMA_WARNING_DISABLE_CLANG(id) _Pragma(PRAGMA_STRINGIFY(clang diagnostic ignored id))
#endif
