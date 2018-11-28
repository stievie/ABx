#pragma once

namespace Game {

class Actor;

namespace Components {

class InputComp
{
private:
    Actor& owner_;
public:
    explicit InputComp(Actor& owner) :
        owner_(owner)
    { }
    ~InputComp() = default;
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
};

}
}
