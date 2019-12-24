#pragma once

#include <cstdlib>
#include <cstddef>
#include <sa/Linkedlist.h>
#include <cassert>
#include <type_traits>
#include <stdint.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

//#define DEBUG_POOLALLOCATOR

namespace sa {

struct PoolInfo
{
    size_t allocs;
    size_t frees;
    size_t current;
    size_t peak;
    size_t used;
    size_t avail;
    unsigned usage;
};

/// Size must be a multiple of ChunkSize. There is no way to get aligned memory.
/// You'd better make sure that T is properly aligned.
template <typename T, size_t ChunkSize>
class PoolAllocator
{
    static_assert(!std::is_constructible<T>::value || std::is_default_constructible<T>::value, "T can only have a default constructor");
private:
    struct FreeHeader { };
    using Node = typename LinkedList<FreeHeader>::Node;
    LinkedList<FreeHeader> freeList_;
    size_t size_;
    void* startPtr_;

    size_t allocs_{ 0 };
    size_t frees_{ 0 };
    size_t peak_{ 0 };

    void* Alloc(const size_t size)
    {
        (void)size;
        // size must be ChunkSize
        assert(size == ChunkSize);
        Node* freePosition = freeList_.pop();
        assert(freePosition != nullptr);
        assert((((uintptr_t)freePosition - (uintptr_t)startPtr_) % ChunkSize) == 0);
        ++allocs_;
        size_t curr = GetCurrentAllocations();
        if (curr > peak_)
            peak_ = curr;
        return (void*)freePosition;
    }

    void Free(void* ptr)
    {
        // Check if this pointer is ours
        assert((uintptr_t)ptr >= (uintptr_t)startPtr_ && (uintptr_t)ptr <= (uintptr_t)startPtr_ + (size_ - ChunkSize));
        // Check if aligned to ChunkSize. If not something is wrong.
        assert((((uintptr_t)ptr - (uintptr_t)startPtr_) % ChunkSize) == 0);
        ++frees_;
        freeList_.push((Node*)ptr);
    }

    void Reset()
    {
        allocs_ = 0;
        frees_ = 0;
        peak_ = 0;

        // Create a linked-list with all free positions
        const size_t nChunks = size_ / ChunkSize;
        for (size_t i = 0; i < nChunks; ++i)
        {
            uintptr_t address = (uintptr_t)startPtr_ + i * ChunkSize;
            freeList_.push((Node*)address);
        }
    }
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    PoolAllocator(size_t size) :
        freeList_{},
        size_(size)
    {
        // Check if ChunkSize is a multiple of size_.
        assert(size_ % ChunkSize == 0);
#ifdef _WIN32
        startPtr_ = VirtualAlloc(0, size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        assert(startPtr_);
#else
        startPtr_ = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, 0, 0);
        assert(startPtr_ != MAP_FAILED);
#endif
        Reset();
    }
    ~PoolAllocator()
    {
#ifdef _WIN32
        VirtualFree(startPtr_, 0, MEM_RELEASE);
#else
        munmap(startPtr_, size_);
#endif
    }

    size_t GetAllocs() const { return allocs_; }
    size_t GetFrees() const { return frees_; }
    size_t GetPeak() const { return peak_; }
    size_t GetCurrentAllocations() const { return allocs_ - frees_; }
    size_t GetUsedMem() const { return GetCurrentAllocations() * ChunkSize; }
    size_t GetAvailMem() const { return size_ - GetUsedMem(); }
    /// Usage, value between 0..100
    unsigned GetUsage() const
    {
        const size_t nChunks = size_ / ChunkSize;
        return static_cast<unsigned>((static_cast<float>(GetCurrentAllocations()) / static_cast<float>(nChunks)) * 100.0f);
    }
    PoolInfo GetInfo() const
    {
        return {
            GetAllocs(),
            GetFrees(),
            GetCurrentAllocations(),
            GetPeak(),
            GetUsedMem(),
            GetAvailMem(),
            GetUsage()
        };
    }

    pointer allocate(size_type n, const void*)
    {
        void* resultMem = Alloc(sizeof(T));
        T* t = nullptr;
        if constexpr (std::is_default_constructible<T>::value)
            t = new(resultMem)T();
        else
            t = reinterpret_cast<T*>(resultMem);

        for (size_type i = 1; i < n; ++i)
        {
            void* oMem = Alloc(sizeof(T));
            if constexpr (std::is_default_constructible<T>::value)
                new(oMem)T();
        }
        return t;
    }
    void deallocate(pointer p, size_type n)
    {
        for (size_type i = 0; i < n; ++i)
        {
            void* ptr = p + (ChunkSize * i);
            if constexpr (std::is_destructible<T>::value)
            {
                T* o = reinterpret_cast<T*>(ptr);
                o->~T();
            }
            Free(ptr);
        }
    }
};

}
