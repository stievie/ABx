#pragma once

#include <AB/Entities/AccountKeyAccounts.h>

namespace DB {

class DBAccountKeyAccounts
{
public:
    DBAccountKeyAccounts() = delete;
    ~DBAccountKeyAccounts() = delete;

    static bool Create(AB::Entities::AccountKeyAccounts& ak);
    static bool Load(AB::Entities::AccountKeyAccounts& ak);
    static bool Save(const AB::Entities::AccountKeyAccounts&);
    static bool Delete(const AB::Entities::AccountKeyAccounts&);
    static bool Exists(const AB::Entities::AccountKeyAccounts& ak);
};

}
