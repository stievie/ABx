#pragma once

#include <AB/Entities/Account.h>

namespace DB {

class DBAccount
{
public:
    DBAccount() = delete;
    ~DBAccount() = delete;

    static bool Create(AB::Entities::Account& account);
    /// Load an account identified by id or name.
    static bool Load(AB::Entities::Account& account);
    static bool Save(const AB::Entities::Account& account);
    static bool Delete(const AB::Entities::Account& account);
    static bool Exists(const AB::Entities::Account& account);
    static bool LogoutAll();
};

}