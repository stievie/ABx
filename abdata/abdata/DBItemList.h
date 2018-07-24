#pragma once

#include <AB/Entities/ItemList.h>

namespace DB {

class DBItemList
{
public:
    DBItemList() = delete;
    ~DBItemList() = delete;

    static bool Create(AB::Entities::ItemList&);
    static bool Load(AB::Entities::ItemList& il);
    static bool Save(const AB::Entities::ItemList&);
    static bool Delete(const AB::Entities::ItemList&);
    static bool Exists(const AB::Entities::ItemList&);
};

}
