#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ITEMS = "game_itemms";

enum ItemType : uint8_t
{
    ItemTypeItem = 0,
    ItemTypeModifier
};

enum ItemID : uint16_t
{
};

struct Item : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ITEMS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_ITEM_NAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(icon, Limits::MAX_FILENAME);
        s.text1b(model, Limits::MAX_FILENAME);
        s.value1b(type);
    }

    uint32_t index = INVALID_INDEX;
    std::string name;
    std::string script;
    std::string icon;
    std::string model;
    ItemType type = ItemTypeUnknown;
};

}
}
