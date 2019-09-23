#pragma once

#include <cstdlib>
#include <cstddef>
#include <sa/Linkedlist.h>
#include <cassert>
#include <type_traits>

namespace sa {

/// Size must be a multiple of ChunkSize
template <typename T, size_t Size, size_t ChunkSize>
class PoolAllocator
{
    static_assert(Size % ChunkSize == 0, "Size must be a multiple of ChunkSize");
private:
    struct FreeHeader { };
    using Node = typename LinkedList<FreeHeader>::Node;
    LinkedList<FreeHeader> freeList_;

    void* startPtr_{ nullptr };
    void* Alloc(const size_t size)
    {
        (void)size;
        // site must be ChunkSize
        assert(size == ChunkSize);
        Node* freePosition = freeList_.pop();
        assert(freePosition != nullptr);
        return (void*)freePosition;
    }

    void Free(void* ptr)
    {
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

    pointer allocate(size_type n, const void*)
    {
        void* resultMem = Alloc(sizeof(T));
        T* t = new(resultMem)T;
        for (size_type i = 1; i < n; ++i)
        {
            void* oMem = Alloc(sizeof(T));
            new(oMem)T;
        }
        return t;
    }
    void deallocate(pointer p, size_type n)
    {
        for (size_type i = 0; i < n; ++i)
        {
            T* o = reinterpret_cast<T*>(p + (ChunkSize * i));
            if constexpr (std::is_trivially_destructible<T>::value)
                o->~T();
            Free(o);
        }
    }
};

}
