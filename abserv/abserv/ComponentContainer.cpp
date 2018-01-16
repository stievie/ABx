#include "stdafx.h"
#include "ComponentContainer.h"

namespace Game {
namespace Components {

void ComponentContainer::Send(const Message& msg)
{
    for (const auto& c : components_)
    {
        c.second->Receive(msg);
    }
}

void ComponentContainer::Update(uint32_t timeElapsed, GameObject& object)
{
    for (const auto& c : components_)
    {
        c.second->Update(timeElapsed, object);
    }
}

}
}
