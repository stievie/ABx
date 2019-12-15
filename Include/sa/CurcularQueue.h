#pragma once

#include <stdint.h>
#include <cassert>
#include <cstddef>
#include <utility>

namespace sa {

// For complex types
template <typename T, size_t Capacity>
class CircularQueue
{
private:
    alignas(T) uint8_t buffer_[sizeof(T) * Capacity];
    size_t size_{ 0 };
    size_t head_{ 0 };
    T* Elements() { return reinterpret_cast<T*>(buffer_); }
    const T* Elements() const { return reinterpret_cast<const T*>(buffer_); }
public:
    ~CircularQueue()
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
        if (size_ == Capacity)
            item.~T();

        new(&item)T(value);
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
        item.~T();
        head_ = (head_ + 1) % Capacity;
        --size_;
        return result;
    }
    void Clear()
    {
        for (size_t i = 0; i < size_; ++i)
            Elements()[(head_ + i) % Capacity].~T();
        head_ = 0;
        size_ = 0;
    }

    size_t Size() const { return size_; }
    bool IsEmpty() const { return size_ == 0; }
    const T& At(size_t index) const
    {
        return Elements()[(head_ + index) % Capacity];
    }
    const T& First() const { return At(0); }
    const T& Last() const { return At(size_ - 1); }

    class ConstIterator
    {
    private:
        friend class CircularQueue;
        const CircularQueue& queue_;
        size_t index_{ 0 };
        ConstIterator(const CircularQueue& queue, size_t index) :
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
    class Iterator
    {
    private:
        friend class CircularQueue;
        CircularQueue& queue_;
        size_t index_{ 0 };
        Iterator(CircularQueue& queue, size_t index) :
            queue_(queue),
            index_(index)
        { }
    public:
        bool operator != (Iterator& other) { return index_ != other.index_; }
        Iterator& operator++()
        {
            index_ = (index_ + 1) % Capacity;
            if (index_ == queue_.head_)
                index_ = queue_.size_;
            return *this;
        }
        T& operator*()
        {
            return queue_.Elements()[index_];
        }
    };

    ConstIterator begin() const { return ConstIterator(*this, head_); }
    ConstIterator end() const { return ConstIterator(*this, size_); }
    Iterator begin() { return Iterator(*this, head_); }
    Iterator end() { return Iterator(*this, size_); }
};

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
        --size_;
        return result;
    }
    void Clear()
    {
        head_ = 0;
        size_ = 0;
    }

    size_t Size() const { return size_; }
    bool IsEmpty() const { return size_ == 0; }
    const T& At(size_t index) const
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
    class Iterator
    {
    private:
        friend class PODCircularQueue;
        PODCircularQueue& queue_;
        size_t index_{ 0 };
        Iterator(PODCircularQueue& queue, size_t index) :
            queue_(queue),
            index_(index)
        { }
    public:
        bool operator != (const Iterator& other) { return index_ != other.index_; }
        Iterator& operator++()
        {
            index_ = (index_ + 1) % Capacity;
            if (index_ == queue_.head_)
                index_ = queue_.size_;
            return *this;
        }
        T& operator*()
        {
            return queue_.Elements()[index_];
        }
    };

    ConstIterator begin() const { return ConstIterator(*this, head_); }
    ConstIterator end() const { return ConstIterator(*this, size_); }
    Iterator begin() { return Iterator(*this, head_); }
    Iterator end() { return Iterator(*this, size_); }
};

}
