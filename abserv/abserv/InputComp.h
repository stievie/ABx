#pragma once

#include "InputQueue.h"

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
    void FollowObject(uint32_t targetId, Net::NetworkMessage& message);
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

    inline void Add(InputType type, const Utils::VariantMap& data)
    {
        inputs_.Add(type, data);
    }
    inline void Add(InputType type)
    {
        inputs_.Add(type, Utils::VariantMapEmpty);
    }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
};

}
}
