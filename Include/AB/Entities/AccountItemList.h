#pragma once

#include <AB/Entities/Entity.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>
#include <AB/Entities/ConcreteItem.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_ACCOUNTT_ITEMLIST = "account_item_list";

struct AccountItemList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTT_ITEMLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(storagePlace);
        s.container(itemUuids, Limits::MAX_ITEMS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    StoragePlace storagePlace = StoragePlaceNone;
    std::vector<std::string> itemUuids;
};

struct ChestItems : AccountItemList
{
    static constexpr const char* KEY()
    {
        return "chest_item_list";
    }
    ChestItems() :
        AccountItemList()
    {
        storagePlace = StoragePlaceChest;
    }
};

}
}
