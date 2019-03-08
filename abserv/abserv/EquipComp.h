#pragma once

#include "Item.h"

namespace Game {

class Actor;

namespace Components {

enum class ItemPos
{
    ArmorHead = 0,
    ArmorChest,
    ArmorHands,
    ArmorLegs,
    ArmorFeet,

    InsigniaHead,
    InsigniaChest,
    InsigniaHands,
    InsigniaLegs,
    InsigniaFeet,

    RuneHead,
    RuneChest,
    RuneHands,
    RuneLegs,
    RuneFeet,

    WeaponLeadHand,
    WeaponOffHand
};

/// Equipment, like Armor, weapons, weapon mods etc.
/// All this stuff adds effects to the actor. These effects are not visible in the clients effects window.
class EquipComp
{
private:
    Actor& owner_;
    std::map<ItemPos, std::unique_ptr<Item>> items_;
public:
    EquipComp() = delete;
    explicit EquipComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    EquipComp(const EquipComp&) = delete;
    EquipComp& operator=(const EquipComp&) = delete;
    ~EquipComp() = default;

    void Update(uint32_t timeElapsed);
};

}
}
