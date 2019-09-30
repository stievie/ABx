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
    void Swap(SharedPtr& other)
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
    bool Empty() const { return !ptr_; }

    /// Copy assignment
    UniquePtr& operator=(const UniquePtr& other) = delete;
    /// Move assignment
    UniquePtr& operator=(UniquePtr&& other)
    {
        UniquePtr tmp = std::move(other);
        std::swap(ptr_, tmp.ptr_);
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
    bool operator==(const UniquePtr& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const UniquePtr& other) const { return ptr_ != other.ptr_; }

    bool operator==(UniquePtr& other) { return ptr_ == other.ptr_; }
    bool operator!=(UniquePtr& other) { return ptr_ != other.ptr_; }

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

template <typename T, typename... ArgTypes>
inline UniquePtr<T> MakeUnique(ArgTypes&& ... Arguments)
{
    return UniquePtr<T>(new T(std::forward<ArgTypes>(Arguments)...));
}

}
