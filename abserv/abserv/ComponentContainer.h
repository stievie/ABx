#pragma once

#include "Component.h"

namespace Game {
namespace Components {

class ComponentContainer
{
private:
    std::map<const char*, std::unique_ptr<Component>> components_;
public:
    ComponentContainer() = default;
    ~ComponentContainer() = default;

    template<typename T>
    T* GetComponent()
    {
        auto it = components_.find(typeid(T).name());
        if (it != components_.end())
            return (*it).second.get();
        return nullptr;
    }
    template<typename T>
    void AddComponent()
    {
        components_.emplace(typeid(T), std::make_unique<T>(this));
    }
    void Send(const Message& msg);
    void Update(uint32_t timeElapsed, GameObject& object);
};

}
}
