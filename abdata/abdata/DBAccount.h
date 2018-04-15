#pragma once

#include "Account.h"

namespace DB {

class DBAccount
{
public:
    DBAccount() = delete;
    ~DBAccount() = delete;

    static bool Load(Entities::Account& account);
    static bool Save(Entities::Account& account);
    static bool Delete(Entities::Account& account);
};

}