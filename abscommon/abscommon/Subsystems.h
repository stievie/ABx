#pragma once

#include <map>

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

        _SubystemWrapper(T* object) :
            object_(std::move(object))
        { }
    };
    std::map<const char*, std::unique_ptr<_Subsystem>> systems_;
public:
    Subsystems() = default;
    ~Subsystems() noexcept
    {
        // Reverse delete subsystems
        std::map<const char*, std::unique_ptr<_Subsystem>>::reverse_iterator itr;
        for (itr = systems_.rbegin(); itr != systems_.rend(); ++itr)
            (*itr).second.reset();
    }

    template<typename T, typename... _CArgs>
    bool CreateSubsystem(_CArgs&&... _Args)
    {
        static const auto key = typeid(T).name();
        auto i = systems_.find(key);
        if (i != systems_.end())
            return false;

        T* system = new T(std::forward<_CArgs>(_Args)...);
        if (system && RegisterSubsystem<T>(system))
            return true;
        if (system)
            delete system;
        return false;
    }

    /// Takes ownership of the object
    template<typename T>
    bool RegisterSubsystem(T* system)
    {
        static const auto key = typeid(T).name();
        auto i = systems_.find(key);
        if (i == systems_.end())
        {
            systems_[key] = std::make_unique<_SubystemWrapper<T>>(system);
            return true;
        }
        return false;
    }

    template<typename T>
    void RemoveSubsystem()
    {
        static const auto key = typeid(T).name();
        auto i = systems_.find(key);
        if (i != systems_.end())
            systems_.erase(i);
    }

    template<typename T>
    T* GetSubsystem()
    {
        static const auto key = typeid(T).name();
        auto i = systems_.find(key);
        if (i != systems_.end())
        {
            auto wrapper = static_cast<_SubystemWrapper<T>*>((*i).second.get());
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
