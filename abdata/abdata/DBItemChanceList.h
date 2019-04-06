#pragma once

#include <AB/Entities/ItemChanceList.h>

namespace DB {

class DBItemChanceList
{
public:
    DBItemChanceList() = delete;
    ~DBItemChanceList() = delete;

    static bool Create(AB::Entities::ItemChanceList& il);
    static bool Load(AB::Entities::ItemChanceList& il);
    static bool Save(const AB::Entities::ItemChanceList& il);
    static bool Delete(const AB::Entities::ItemChanceList& il);
    static bool Exists(const AB::Entities::ItemChanceList& il);
};

}
