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


/// Loose implementation of a SharedPtr. Unintrusive and unflexible. No WeakPtr,
/// no pointer cast.
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
            refs_ = new int;
            *refs_ = 0;
        }
        ++(*refs_);
    }
    void DecRef()
    {
        if (!refs_)
            return;
        --(*refs_);
    }
    template <typename U>
    void Swap(SharedPtr<U>& other)
    {
       std::swap(ptr_, other.ptr_);
       // Pinch the counter of the other guy
       std::swap(refs_, other.refs_);
    }
public:
    /// You'll get an empty pointer
    SharedPtr() = default;
    SharedPtr(std::nullptr_t) { }
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

        DecRef();
        if (*refs_ == 0)
        {
            if (ptr_)
                DefaultDelete<T>()(ptr_);
            // Since we are the last and delete the pointer, we must also delete
            // the counter
            delete refs_;
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

        DecRef();
        if (*refs_ == 0)
        {
            if (ptr_)
            {
                DefaultDelete<T>()(ptr_);
                ptr_ = nullptr;
            }
            // Since we are the last and delete the pointer, we must also delete
            // the counter
            delete refs_;
            refs_ = nullptr;
        }
    }
    /// Check if it contains a pointer
    bool Empty() const { return !ptr_; }
    /// Get number of references
    int Refs() const { return refs_ ? *refs_ : 0; }

    /// Move assignment
    SharedPtr& operator=(SharedPtr&& other)
    {
        SharedPtr tmp = std::move(other);
        Swap(tmp);
        return *this;
    }

    /// Copy assignment
    SharedPtr& operator=(const SharedPtr& other)
    {
        SharedPtr tmp = other;
        Swap(tmp);
        return *this;
    }

    /// Get raw pointer
    T* Ptr()
    {
        assert(ptr_);
        return ptr_;
    }
    const T* Ptr() const
    {
        assert(ptr_);
        return ptr_;
    }
    T* operator->()
    {
        assert(ptr_);
        return ptr_;
    }
    const T* operator->() const
    {
        assert(ptr_);
        return ptr_;
    }
    T& operator *()
    {
        assert(ptr_);
        return *ptr_;
    }
    const T& operator *() const
    {
        assert(ptr_);
        return *ptr_;
    }

    operator bool() { return !!ptr_; }
    bool operator!() const { return !ptr_; }
    bool operator==(std::nullptr_t) const { return !ptr_; }
    bool operator!=(std::nullptr_t) const { return ptr_; }
    bool operator==(const SharedPtr& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const SharedPtr& other) const { return ptr_ != other.ptr_; }

    bool operator==(SharedPtr& other) { return ptr_ == other.ptr_; }
    bool operator!=(SharedPtr& other) { return ptr_ != other.ptr_; }

    bool operator==(const T* other) const { return ptr_ == other; }
    bool operator!=(const T* other) const { return ptr_ != other; }

    bool operator==(T* other) { return ptr_ == other; }
    bool operator!=(T* other) { return ptr_ != other; }
};

template <typename T, typename... ArgTypes>
inline SharedPtr<T> MakeShared(ArgTypes&& ... Arguments)
{
    return SharedPtr<T>(new T(std::forward<ArgTypes>(Arguments)...));
}

}
