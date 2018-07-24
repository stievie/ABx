#pragma once

#include <AB/Entities/Item.h>

namespace DB {

class DBItem
{
public:
    DBItem() = delete;
    ~DBItem() = delete;

    static bool Create(AB::Entities::Item& item);
    static bool Load(AB::Entities::Item& item);
    static bool Save(const AB::Entities::Item& item);
    static bool Delete(const AB::Entities::Item& item);
    static bool Exists(const AB::Entities::Item& item);
};

}
