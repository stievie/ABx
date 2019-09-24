#pragma once

#include <cstdlib>
#include <cstddef>
#include <sa/Linkedlist.h>
#include <cassert>
#include <type_traits>

//#define DEBUG_POOLALLOCATOR

namespace sa {

#ifdef DEBUG_POOLALLOCATOR
struct PoolInfo
{
    size_t allocs;
    size_t frees;
    size_t current;
    size_t used;
    size_t avail;
};
#endif

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
#ifdef DEBUG_POOLALLOCATOR
    size_t allocs_{ 0 };
    size_t frees_{ 0 };
#endif

    void* startPtr_{ nullptr };
    void* Alloc(const size_t size)
    {
        (void)size;
        // size must be ChunkSize
        assert(size == ChunkSize);
        Node* freePosition = freeList_.pop();
        assert(freePosition != nullptr);
#ifdef DEBUG_POOLALLOCATOR
        ++allocs_;
#endif
        return (void*)freePosition;
    }

    void Free(void* ptr)
    {
#ifdef DEBUG_POOLALLOCATOR
        ++frees_;
#endif
        freeList_.push((Node*)ptr);
    }

    void Reset()
    {
        // Create a linked-list with all free positions
        const size_t nChunks = Size / ChunkSize;
        for (size_t i = 0; i < nChunks; ++i)
        {
            size_t address = (size_t)startPtr_ + i * ChunkSize;
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

#ifdef DEBUG_POOLALLOCATOR
    size_t GetAllocs() const { return allocs_; }
    size_t GetFrees() const { return frees_; }
    size_t GetCurrentAllocations() const { return allocs_ - frees_; }
    size_t GetUsedMem() const { return GetCurrentAllocations() * ChunkSize; }
    size_t GetAvailMem() const { return Size - GetUsedMem(); }
    PoolInfo GetInfo() const
    {
        return {
            GetAllocs(),
            GetFrees(),
            GetCurrentAllocations(),
            GetUsedMem(),
            GetAvailMem()
        };
    }
#endif

    pointer allocate(size_type n, const void*)
    {
        void* resultMem = Alloc(sizeof(T));
        T* t = nullptr;
        if constexpr (std::is_default_constructible<T>::value)
            t = new(resultMem)T;
        else
            t = reinterpret_cast<T*>(resultMem);

        for (size_type i = 1; i < n; ++i)
        {
            void* oMem = Alloc(sizeof(T));
            if constexpr (std::is_default_constructible<T>::value)
                new(oMem)T;
        }
        return t;
    }
    void deallocate(pointer p, size_type n)
    {
        for (size_type i = 0; i < n; ++i)
        {
            T* o = reinterpret_cast<T*>(p + (ChunkSize * i));
            if constexpr (std::is_destructible<T>::value)
                o->~T();
            Free(o);
        }
    }
};

}
