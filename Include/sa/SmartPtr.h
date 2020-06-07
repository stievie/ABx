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

#include <utility>
#include <cassert>

namespace sa {

/// Default deleter
template <typename T>
struct DefaultDelete
{
    DefaultDelete() = default;
    void operator()(T* p) const noexcept
    {
        delete p;
    }
};

/// Loose implementation of a SharedPtr. Unintrusive and unflexible.
/// Limitations:
/// * No WeakPtr,
/// * no pointer cast,
/// * _not_ thread safe.
/// * You can not create more SharedPtr's from the same pointer. You can't do that with a std::shared_ptr either.
/// Anyway, sometimes you don't need all these features, which can make stuff complicated.
template <typename T>
class SharedPtr
{
private:
    T* ptr_{ nullptr };
    mutable int* refs_{ nullptr };

    void AddRef()
    {
        if (!refs_)
        {
            // There seems to be some trouble with EASTLs allocator
            refs_ = (int*)malloc(sizeof(int));
            *refs_ = 0;
        }
        ++(*refs_);
    }
    void Release()
    {
        if (!refs_)
            return;
        --(*refs_);
    }
    void Swap(SharedPtr& other) noexcept
    {
       std::swap(ptr_, other.ptr_);
       // Pinch the counter of the other guy
       std::swap(refs_, other.refs_);
    }
public:
    /// You'll get an empty pointer
    constexpr SharedPtr() noexcept = default;
    constexpr SharedPtr(std::nullptr_t) noexcept { }
    /// Construct from raw pointer
    SharedPtr(const T* ptr) :
        ptr_(const_cast<T*>(ptr))
    {
        if (ptr_)
            AddRef();
    }
    /// Copy constructor
    SharedPtr(const SharedPtr& other) :
        ptr_(const_cast<T*>(other.Ptr()))
    {
        if (ptr_)
        {
            // Both have the same counter
            refs_ = other.refs_;
            AddRef();
        }
    }
    /// Move constructor
    SharedPtr(SharedPtr&& other) noexcept :
        ptr_(other.ptr_)
    {
        // We get the counter
        std::swap(refs_, other.refs_);
    }

    ~SharedPtr()
    {
        if (!refs_)
            // Someone pinched our counter, so we are not longer responsible for
            // the pointer
            return;

        Release();
        if (*refs_ == 0)
        {
            if (ptr_)
                DefaultDelete<T>()(ptr_);
            // Since we are the last and delete the pointer, we must also delete
            // the counter
            free(refs_);
        }
    }

    void Reset()
    {
        if (!refs_)
        {
            // Someone pinched our counter, so we are not longer responsible for
            // the pointer
            ptr_ = nullptr;
            return;
        }

        Release();
        if (*refs_ == 0)
        {
            if (ptr_)
            {
                DefaultDelete<T>()(ptr_);
                ptr_ = nullptr;
            }
            // Since we are the last and delete the pointer, we must also delete
            // the counter
            free(refs_);
            refs_ = nullptr;
        }
    }
    /// Check if it contains a pointer
    bool Empty() const noexcept { return !ptr_; }
    /// Get number of references
    int Refs() const noexcept { return refs_ ? *refs_ : 0; }

    /// Move assignment
    SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        if (&other != this)
        {
            SharedPtr tmp = std::move(other);
            Swap(tmp);
        }
        return *this;
    }

    /// Copy assignment
    SharedPtr& operator=(const SharedPtr& other) noexcept
    {
        if (&other != this)
        {
            SharedPtr tmp = other;
            Swap(tmp);
        }
        return *this;
    }

    /// Get raw pointer
    T* Ptr() noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    const T* Ptr() const noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    T* operator->() noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    const T* operator->() const noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    T& operator *() noexcept
    {
        assert(ptr_);
        return *ptr_;
    }
    const T& operator *() const noexcept
    {
        assert(ptr_);
        return *ptr_;
    }

    operator bool() noexcept { return !!ptr_; }
    bool operator!() const noexcept { return !ptr_; }
    bool operator==(std::nullptr_t) const noexcept { return !ptr_; }
    bool operator!=(std::nullptr_t) const noexcept { return ptr_; }
    bool operator==(const SharedPtr& other) const noexcept { return ptr_ == other.ptr_; }
    bool operator!=(const SharedPtr& other) const noexcept { return ptr_ != other.ptr_; }

    bool operator==(SharedPtr& other) noexcept { return ptr_ == other.ptr_; }
    bool operator!=(SharedPtr& other) noexcept { return ptr_ != other.ptr_; }

    bool operator==(const T* other) const noexcept { return ptr_ == other; }
    bool operator!=(const T* other) const noexcept { return ptr_ != other; }

    bool operator==(T* other) noexcept { return ptr_ == other; }
    bool operator!=(T* other) noexcept { return ptr_ != other; }
};

template <typename T>
class UniquePtr
{
private:
    T* ptr_{ nullptr };
public:
    /// You'll get an empty pointer
    constexpr UniquePtr() noexcept = default;
    constexpr UniquePtr(std::nullptr_t) noexcept { }
    /// Construct from raw pointer
    UniquePtr(const T* ptr) :
        ptr_(const_cast<T*>(ptr))
    { }
    /// Copy constructor
    UniquePtr(const UniquePtr& other) = delete;
    /// Move constructor
    UniquePtr(UniquePtr&& other) noexcept
    {
        // We get the pointer
        std::swap(ptr_, other.ptr_);
    }

    ~UniquePtr()
    {
        if (ptr_)
            DefaultDelete<T>()(ptr_);
    }

    void Reset()
    {
        if (ptr_)
        {
            DefaultDelete<T>()(ptr_);
            ptr_ = nullptr;
        }
    }
    /// Check if it contains a pointer
    bool Empty() const noexcept { return !ptr_; }

    /// Copy assignment
    UniquePtr& operator=(const UniquePtr& other) = delete;
    /// Move assignment
    UniquePtr& operator=(UniquePtr&& other) noexcept
    {
        if (&other != this)
        {
            UniquePtr tmp = std::move(other);
            std::swap(ptr_, tmp.ptr_);
        }
        return *this;
    }

    /// Get raw pointer
    T* Ptr() noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    const T* Ptr() const noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    T* operator->() noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    const T* operator->() const noexcept
    {
        assert(ptr_);
        return ptr_;
    }
    T& operator *() noexcept
    {
        assert(ptr_);
        return *ptr_;
    }
    const T& operator *() const noexcept
    {
        assert(ptr_);
        return *ptr_;
    }

    operator bool() noexcept { return !!ptr_; }
    bool operator!() const noexcept { return !ptr_; }
    bool operator==(std::nullptr_t) const noexcept { return !ptr_; }
    bool operator!=(std::nullptr_t) const noexcept { return ptr_; }
    bool operator==(const UniquePtr& other) const noexcept { return ptr_ == other.ptr_; }
    bool operator!=(const UniquePtr& other) const noexcept { return ptr_ != other.ptr_; }

    bool operator==(UniquePtr& other) noexcept { return ptr_ == other.ptr_; }
    bool operator!=(UniquePtr& other) noexcept { return ptr_ != other.ptr_; }

    bool operator==(const T* other) const noexcept { return ptr_ == other; }
    bool operator!=(const T* other) const noexcept { return ptr_ != other; }

    bool operator==(T* other) noexcept { return ptr_ == other; }
    bool operator!=(T* other) noexcept { return ptr_ != other; }
};

template <typename T, typename... ArgTypes>
inline SharedPtr<T> MakeShared(ArgTypes&& ... Arguments)
{
    return SharedPtr<T>(new T(std::forward<ArgTypes>(Arguments)...));
}

template <typename T, typename... ArgTypes>
inline UniquePtr<T> MakeUnique(ArgTypes&& ... Arguments)
{
    return UniquePtr<T>(new T(std::forward<ArgTypes>(Arguments)...));
}

}
