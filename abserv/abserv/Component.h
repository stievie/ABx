#pragma once

#include "Variant.h"

namespace Game {

class GameObject;

namespace Components {

class ComponentContainer;

// TODO: Preallocate massages
typedef size_t msg_t;
struct Message
{
    msg_t msg;
    Utils::VariantMap data;
};

class Component
{
private:
    ComponentContainer* container_;
public:
    Component(ComponentContainer* container) :
        container_(container)
    {}
    virtual ~Component();

    virtual void Update(uint32_t timeElapsed, GameObject& object) = 0;
    virtual void Receive(const Message& msg) = 0;
};

}
}

