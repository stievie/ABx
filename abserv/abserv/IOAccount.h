#pragma once

#include "Account.h"

namespace DB {

class IOAccount
{
public:
    static bool LoginServerAuth(const std::string& name, const std::string& pass, Account& account);
    static bool Save(const Account& account);
};

}
