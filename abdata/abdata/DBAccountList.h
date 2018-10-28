#pragma once

#include <AB/Entities/AccountList.h>

namespace DB {

class DBAccountList
{
public:
    DBAccountList() = delete;
    ~DBAccountList() = delete;

    static bool Create(AB::Entities::AccountList&);
    static bool Load(AB::Entities::AccountList& al);
    static bool Save(const AB::Entities::AccountList&);
    static bool Delete(const AB::Entities::AccountList&);
    static bool Exists(const AB::Entities::AccountList&);
};

}
