#pragma once

#include <AB/Entities/AccountItemList.h>

namespace DB {

class DBAccountItemList
{
public:
    DBAccountItemList() = delete;
    ~DBAccountItemList() = delete;

    static bool Create(AB::Entities::AccountItemList& li);
    static bool Load(AB::Entities::AccountItemList& il);
    static bool Save(const AB::Entities::AccountItemList& il);
    static bool Delete(const AB::Entities::AccountItemList& il);
    static bool Exists(const AB::Entities::AccountItemList& il);
};

}
