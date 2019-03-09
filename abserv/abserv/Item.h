#pragma once

#include <AB/Entities/Item.h>
#include "Script.h"

namespace Game {

enum class ItemUpgrade
{
    Pefix = 0,           // Insignia for armor, prefix for weapon
    Suffix,              // Rune for armor, suffix for weapon
    Inscription          // For lead/off hand weapons
};

class Item
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionGetDamage = 1 << 1,
    };
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    uint32_t functions_;
    std::map<ItemUpgrade, std::unique_ptr<Item>> upgrades_;
    void InitializeLua();
    bool HaveFunction(Function func)
    {
        return (functions_ & func) == func;
    }
public:
    static void RegisterLua(kaguya::State& state);

    Item() = delete;
    explicit Item(const AB::Entities::Item& item) :
        functions_(FunctionNone),
        data_(item)
    {
        InitializeLua();
    }
    // non-copyable
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;

    ~Item() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);
    /// Upgrade this item
    void SetUpgrade(ItemUpgrade type, uint32_t index);
    void RemoveUpgrade(ItemUpgrade type);

    AB::Entities::Item data_;
};

}
