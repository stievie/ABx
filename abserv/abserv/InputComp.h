#pragma once

#include "InputQueue.h"

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class InputComp
{
private:
    Actor& owner_;
    InputQueue inputs_;
    void SelectObject(uint32_t sourceId, uint32_t targetId, Net::NetworkMessage& message);
    void ClickObject(uint32_t sourceId, uint32_t targetId, Net::NetworkMessage& message);
    void FollowObject(uint32_t targetId, bool ping, Net::NetworkMessage& message);
public:
    InputComp() = delete;
    explicit InputComp(Actor& owner) :
        owner_(owner),
        inputs_()
    { }
    // non-copyable
    InputComp(const InputComp&) = delete;
    InputComp& operator=(const InputComp&) = delete;
    ~InputComp() = default;

    void Add(InputType type, Utils::VariantMap&& data)
    {
        inputs_.Add(type, std::move(data));
    }
    void Add(InputType type)
    {
        inputs_.Add(type);
    }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
};

}
}
