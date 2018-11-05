#pragma once

#include <AB/Entities/AccountKeyList.h>

namespace DB {

class DBAccountKeyList
{
public:
    DBAccountKeyList() = delete;
    ~DBAccountKeyList() = delete;

    static bool Create(AB::Entities::AccountKeyList&);
    static bool Load(AB::Entities::AccountKeyList& al);
    static bool Save(const AB::Entities::AccountKeyList&);
    static bool Delete(const AB::Entities::AccountKeyList&);
    static bool Exists(const AB::Entities::AccountKeyList&);
};

}
