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
    int result = posix_memalign(&p, alignment, size);
    (void)result;
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

void* operator new[](size_t size, size_t alignment, const std::nothrow_t&) EA_THROW_SPEC_NEW_NONE()
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
