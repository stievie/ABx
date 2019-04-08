#pragma once

#include <AB/Entities/TypedItemList.h>

namespace DB {

class DBTypedItemList
{
public:
    DBTypedItemList() = delete;
    ~DBTypedItemList() = delete;

    static bool Create(AB::Entities::TypedItemList& il);
    static bool Load(AB::Entities::TypedItemList& il);
    static bool Save(const AB::Entities::TypedItemList& il);
    static bool Delete(const AB::Entities::TypedItemList& il);
    static bool Exists(const AB::Entities::TypedItemList& il);
};

}
