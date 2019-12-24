#pragma once

#include <unordered_map>
#include <vector>
#include <cassert>

#include <sa/TypeName.h>
#include <sa/StringHash.h>

class Subsystems
{
private:
    class _Subsystem
    { };
    template<typename T>
    class _SubystemWrapper : public _Subsystem
    {
    public:
        std::unique_ptr<T> object_;
        explicit _SubystemWrapper(T* object) :
            object_(std::move(object))
        { }
    };
    std::unordered_map<size_t, std::unique_ptr<_Subsystem>> systems_;
    /// The creation order of the systems
    std::vector<size_t> order_;
public:
    Subsystems() = default;
    ~Subsystems() noexcept
    {
        Clear();
    }

    void Clear() noexcept
    {
        // Reverse delete subsystems
        std::vector<size_t>::reverse_iterator itr;
        for (itr = order_.rbegin(); itr != order_.rend(); ++itr)
        {
            auto sit = systems_.find(*itr);
            assert(sit != systems_.end());
            (*sit).second.reset();
        }
        systems_.clear();
        order_.clear();
    }

    template<typename T, typename... _CArgs>
    bool CreateSubsystem(_CArgs&&... _Args)
    {
        static constexpr size_t key = sa::StringHash(sa::TypeName<T>::Get());
        const auto i = systems_.find(key);
        if (i != systems_.end())
            return false;

        T* system = new T(std::forward<_CArgs>(_Args)...);
        if (system)
        {
            if (RegisterSubsystem<T>(system))
                return true;
            delete system;
        }
        return false;
    }

    /// Takes ownership of the object
    template<typename T>
    bool RegisterSubsystem(T* system)
    {
        static constexpr size_t key = sa::StringHash(sa::TypeName<T>::Get());
        const auto i = systems_.find(key);
        if (i == systems_.end())
        {
            systems_[key] = std::make_unique<_SubystemWrapper<T>>(system);
            order_.push_back(key);
            return true;
        }
        return false;
    }

    template<typename T>
    void RemoveSubsystem()
    {
        static constexpr size_t key = sa::StringHash(sa::TypeName<T>::Get());
        const auto i = systems_.find(key);
        if (i != systems_.end())
        {
            auto it = std::find_if(order_.begin(), order_.end(), [&](size_t current) -> bool {
                return current == key;
            });
            assert(it != order_.end());
            order_.erase(it);

            systems_.erase(i);
        }
    }

    template<typename T>
    T* GetSubsystem()
    {
        static constexpr size_t key = sa::StringHash(sa::TypeName<T>::Get());
        const auto i = systems_.find(key);
        if (i != systems_.end())
        {
            const auto wrapper = static_cast<_SubystemWrapper<T>*>((*i).second.get());
            return wrapper->object_.get();
        }
        return nullptr;
    }

    static Subsystems Instance;
};

template<typename T>
inline T* GetSubsystem()
{
    return Subsystems::Instance.GetSubsystem<T>();
}
