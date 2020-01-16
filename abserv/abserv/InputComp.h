#pragma once

#include "InputQueue.h"
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class InputComp
{
    NON_COPYABLE(InputComp)
private:
    Actor& owner_;
    InputQueue inputs_;
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
public:
    InputComp() = delete;
    explicit InputComp(Actor& owner) :
        owner_(owner),
        inputs_()
    { }
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
