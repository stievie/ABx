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
    ~Subsystems() = default;

    template<typename T, typename... _CArgs>
    T* CreateSubsystem(_CArgs&&... _Args)
    {
        auto i = systems_.find(typeid(T).name());
        if (i != systems_.end())
            return nullptr;

        T* system = new T(std::forward<_CArgs>(_Args)...);
        RegisterSubsystem<T>(system);
        return system;
    }

    /// Takes ownership of the object
    template<typename T>
    void RegisterSubsystem(T* system)
    {
        auto i = systems_.find(typeid(T).name());
        if (i == systems_.end())
        {
            systems_[typeid(T).name()] = std::make_unique<_SubystemWrapper<T>>(system);
        }
    }

    template<typename T>
    void RemoveSubsystem()
    {
        auto i = systems_.find(typeid(T).name());
        if (i != systems_.end())
            systems_.erase(i);
    }

    template<typename T>
    T* GetSubsystem()
    {
        auto i = systems_.find(typeid(T).name());
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
