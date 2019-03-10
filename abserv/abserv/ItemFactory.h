#pragma once

#include "Item.h"

namespace Game {

class ItemFactory
{
public:
    ItemFactory();
    ~ItemFactory() = default;

    std::unique_ptr<Item> CreateItem(const std::string& itemUuid);
};

}
