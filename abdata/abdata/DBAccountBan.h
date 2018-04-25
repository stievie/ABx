#pragma once

#include <AB/Entities/AccountBan.h>

namespace DB {

class DBAccountBan
{
public:
    DBAccountBan() = delete;
    ~DBAccountBan() = delete;

    static bool Create(AB::Entities::AccountBan& ban);
    static bool Load(AB::Entities::AccountBan& ban);
    static bool Save(const AB::Entities::AccountBan& ban);
    static bool Delete(const AB::Entities::AccountBan& ban);
};

}
