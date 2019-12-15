#pragma once

#include <stdint.h>
#include <cassert>

namespace sa {

/// Only for POD types
template <typename T, size_t Capacity>
class PODCircularQueue
{
private:
    alignas(T) uint8_t buffer_[sizeof(T) * Capacity];
    size_t size_{ 0 };
    size_t head_{ 0 };
    T* Elements() { return reinterpret_cast<T*>(buffer_); }
    const T* Elements() const { return reinterpret_cast<const T*>(buffer_); }
public:
    ~PODCircularQueue()
    {
        Clear();
    }
    void Enqueue(const T& value)
    {
        Enqueue(std::move(T(value)));
    }
    void Enqueue(T&& value)
    {
        auto& item = Elements()[(head_ + size_) % Capacity];
        item = std::move(value);
        if (size_ == Capacity)
            head_ = (head_ + 1) % Capacity;
        else
            ++size_;
    }
    T Dequeue()
    {
        assert(!IsEmpty());
        auto& item = Elements()[head_];
        T result = std::move(item);
        head_ = (head_ + 1) % Capacity;
        --size;
        return result;
    }
    void Clear()
    {
        head_ = 0;
        size_ = 0;
    }

    size_t Size() const { return size_; }
    bool IsEmpty() const { return size_ == 0; }
    const T& At(size_t index)
    {
        return Elements()[(head_ + index) % Capacity];
    }
    const T& First() const { return At(0); }
    const T& Last() const { return At(size_ - 1); }

    class ConstIterator
    {
    private:
        friend class PODCircularQueue;
        const PODCircularQueue& queue_;
        size_t index_{ 0 };
        ConstIterator(const PODCircularQueue& queue, size_t index) :
            queue_(queue),
            index_(index)
        { }
    public:
        bool operator != (const ConstIterator& other) { return index_ != other.index_; }
        ConstIterator& operator++()
        {
            index_ = (index_ + 1) % Capacity;
            if (index_ == queue_.head_)
                index_ = queue_.size_;
            return *this;
        }
        const T& operator*() const
        {
            return queue_.Elements()[index_];
        }
    };

    ConstIterator Begin() const { return ConstIterator(*this, head_); }
    ConstIterator End() const { return ConstIterator(*this, size_); }
};

}
