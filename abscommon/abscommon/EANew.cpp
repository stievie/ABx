#include "stdafx.h"
#include "EANew.h"

// https://github.com/electronicarts/EASTL/blob/master/test/source/EASTLTestAllocator.cpp

#if !defined(EA_PLATFORM_MICROSOFT) || defined(EA_PLATFORM_MINGW)
#include <stdlib.h>
#endif

namespace Internal
{
void* EASTLAlignedAlloc(size_t size, size_t alignment)
{
#ifdef EA_PLATFORM_MICROSOFT
    return _aligned_malloc(size, alignment);
#else
    // aligned_malloc() causes:
    // malloc.c:2379: sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) || ((unsigned long) (old_size) >= MINSIZE && prev_inuse (old_top) && ((unsigned long) old_end & (pagesize - 1)) == 0)' failed.
    //return aligned_alloc(size, alignment);

    // So let's take the EA ay.
    void* p = nullptr;
    alignment = alignment < sizeof(void*) ? sizeof(void*) : alignment;
    posix_memalign(&p, alignment, size);
    return p;
#endif
}

void EASTLAlignedFree(void* p)
{
#ifdef EA_PLATFORM_MICROSOFT
    _aligned_free(p);
#else
    free(p);
#endif
}
}

void* operator new(size_t size)
{
    return Internal::EASTLAlignedAlloc(size, 16);
}

void* operator new[](size_t size)
{
    return Internal::EASTLAlignedAlloc(size, 16);
}

void* operator new[](size_t size, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
    return Internal::EASTLAlignedAlloc(size, 16);
}

void* operator new[](size_t size, size_t alignment, size_t /*alignmentOffset*/, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
    return Internal::EASTLAlignedAlloc(size, alignment);
}

void* operator new(size_t size, size_t alignment)
{
    return Internal::EASTLAlignedAlloc(size, alignment);
}

void* operator new(size_t size, size_t alignment, const std::nothrow_t&) EA_THROW_SPEC_NEW_NONE()
{
    return Internal::EASTLAlignedAlloc(size, alignment);
}

void* operator new[](size_t size, size_t alignment)
{
    return Internal::EASTLAlignedAlloc(size, alignment);
}

void* operator new[](size_t size, size_t alignment, const std::nothrow_t&)EA_THROW_SPEC_NEW_NONE()
{
    return Internal::EASTLAlignedAlloc(size, alignment);
}

// C++14 deleter
void operator delete(void* p, std::size_t sz) EA_THROW_SPEC_DELETE_NONE()
{
    Internal::EASTLAlignedFree(p); EA_UNUSED(sz);
}

void operator delete[](void* p, std::size_t sz) EA_THROW_SPEC_DELETE_NONE()
{
    Internal::EASTLAlignedFree(p); EA_UNUSED(sz);
}

void operator delete(void* p) EA_THROW_SPEC_DELETE_NONE()
{
    Internal::EASTLAlignedFree(p);
}

void operator delete[](void* p) EA_THROW_SPEC_DELETE_NONE()
{
    Internal::EASTLAlignedFree(p);
}
