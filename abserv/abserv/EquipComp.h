#pragma once

namespace Game {

class Actor;

namespace Components {

class EquipComp
{
private:
    Actor& owner_;
public:
    explicit EquipComp(Actor& owner) :
        owner_(owner)
    { }
    ~EquipComp() = default;
    void Update(uint32_t timeElapsed);
};

}
}
