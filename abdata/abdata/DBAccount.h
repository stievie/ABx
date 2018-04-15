#pragma once

#include <AB/Entities/Account.h>

namespace DB {

class DBAccount
{
public:
    DBAccount() = delete;
    ~DBAccount() = delete;

    static bool Load(AB::Entities::Account& account);
    static bool Save(AB::Entities::Account& account);
    static bool Delete(AB::Entities::Account& account);
};

}