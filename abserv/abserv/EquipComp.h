#pragma once

namespace Game {

class Actor;

namespace Components {

/// Equipment, like Armor, weapons, weapon mods etc.
/// All this stuff adds effects to the actor. These effects are not visible in the clients effects window.
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
