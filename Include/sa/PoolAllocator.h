#pragma once

#include <cstdlib>
#include <cstddef>
#include <sa/Linkedlist.h>
#include <cassert>
#include <type_traits>
#include <stdint.h>

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

/// Size must be a multiple of ChunkSize
template <typename T, size_t Size, size_t ChunkSize>
class PoolAllocator
{
    static_assert(Size % ChunkSize == 0, "Size must be a multiple of ChunkSize");
    static_assert(!std::is_constructible<T>::value || std::is_default_constructible<T>::value, "T can only have a default constructor");
private:
    struct FreeHeader { };
    using Node = typename LinkedList<FreeHeader>::Node;
    LinkedList<FreeHeader> freeList_;
    size_t allocs_{ 0 };
    size_t frees_{ 0 };
    size_t peak_{ 0 };

    void* startPtr_{ nullptr };
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
        assert((uintptr_t)ptr >= (uintptr_t)startPtr_ && (uintptr_t)ptr <= (uintptr_t)startPtr_ + (Size - ChunkSize));
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
        static constexpr size_t nChunks = Size / ChunkSize;
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

    PoolAllocator() :
        freeList_{}
    {
        startPtr_ = malloc(Size);
        Reset();
    }
    ~PoolAllocator()
    {
        free(startPtr_);
    }

    size_t GetAllocs() const { return allocs_; }
    size_t GetFrees() const { return frees_; }
    size_t GetPeak() const { return peak_; }
    size_t GetCurrentAllocations() const { return allocs_ - frees_; }
    size_t GetUsedMem() const { return GetCurrentAllocations() * ChunkSize; }
    size_t GetAvailMem() const { return Size - GetUsedMem(); }
    /// Usage, value between 0..100
    unsigned GetUsage() const {
        static constexpr size_t nChunks = Size / ChunkSize;
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
