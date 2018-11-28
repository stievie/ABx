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
public:
    explicit InputComp(Actor& owner) :
        owner_(owner),
        inputs_()
    { }
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
