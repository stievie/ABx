#pragma once

#include "Item.h"
#include "Damage.h"

namespace Game {

class Actor;

namespace Components {

/// Equipment, like Armor, weapons, weapon mods etc.
class EquipComp
{
private:
    Actor& owner_;
    std::map<EquipPos, std::unique_ptr<Item>> items_;
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

    void SetItem(std::unique_ptr<Item> item);
    Item* GetItem(EquipPos pos) const;
    std::vector<Item*> GetItems() const;

    void RemoveItem(EquipPos pos);
    void SetUpgrade(EquipPos pos, ItemUpgrade type, uint32_t index);
    void RemoveUpgrade(EquipPos pos, ItemUpgrade type);

    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
    uint32_t GetAttributeValue(uint32_t index);

    template<typename Func>
    void VisitItems(Func&& func)
    {
        for (const auto& o : items_)
        {
            func(o.second.get());
        }
    }

};

}
}
