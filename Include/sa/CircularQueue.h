/**
 * Copyright 2017-2020 Stefan Ascher
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

#pragma once

#include <stdint.h>
#include <cassert>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <sa/Assert.h>

namespace sa {

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
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    ~CircularQueue()
    {
        Clear();
    }
    template<typename U = T,
        std::enable_if_t<std::is_fundamental<U>::value, int> = 0>
    void Enqueue(T value)
    {
        static_assert(std::is_fundamental<T>::value, "Not a fundamental type");
        Elements()[(head_ + size_) % Capacity] = value;
        if (size_ == Capacity)
            head_ = (head_ + 1) % Capacity;
        else
            ++size_;
    }
    template<typename U = T,
        std::enable_if_t<std::is_compound<U>::value, int> = 0>
    void Enqueue(const T& value)
    {
        static_assert(std::is_compound<T>::value, "Not a compound type");
        Enqueue(std::move(T(value)));
    }
    template<typename U = T,
        std::enable_if_t<std::is_compound<U>::value, int> = 0>
    void Enqueue(T&& value)
    {
        static_assert(std::is_compound<T>::value, "Not a compound type");
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
        ASSERT(!IsEmpty());
        if constexpr (std::is_compound<T>::value)
        {
            auto& item = Elements()[head_];
            T result = std::move(item);
            item.~T();
            head_ = (head_ + 1) % Capacity;
            --size_;
            return result;
        }
        else
        {
            T result = Elements()[head_];
            head_ = (head_ + 1) % Capacity;
            --size_;
            return result;
        }
    }
    void Clear()
    {
        if constexpr (std::is_compound<T>::value)
        {
            for (size_t i = 0; i < size_; ++i)
                Elements()[(head_ + i) % Capacity].~T();
        }
        head_ = 0;
        size_ = 0;
    }

    size_t Size() const { return size_; }
    bool IsEmpty() const { return size_ == 0; }
    [[nodiscard]] const T& At(size_t index) const
    {
        return Elements()[(head_ + index) % Capacity];
    }
    [[nodiscard]] const T& First() const { return At(0); }
    [[nodiscard]] const T& Last() const { return At(size_ - 1); }

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

}
