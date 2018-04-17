#pragma once

#include <AB/Entities/Account.h>

namespace DB {

class DBAccount
{
public:
    DBAccount() = delete;
    ~DBAccount() = delete;

    static uint32_t Create(AB::Entities::Account& account);
    static bool Load(AB::Entities::Account& account);
    static bool Save(const AB::Entities::Account& account);
    static bool Delete(const AB::Entities::Account& account);
};

}