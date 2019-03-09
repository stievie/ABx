#pragma once

#include <AB/Entities/Item.h>

namespace IO {

class IOItem
{
public:
    IOItem() = delete;

    static bool LoadItem(AB::Entities::Item& item);
    static bool LoadItemByIndex(AB::Entities::Item& item, uint32_t index);
};

}
