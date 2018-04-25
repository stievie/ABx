#pragma once

#include <AB/Entities/IpBan.h>

namespace DB {

class DBIpBan
{
public:
    DBIpBan() = delete;
    ~DBIpBan() = delete;

    static bool Create(AB::Entities::IpBan& ban);
    static bool Load(AB::Entities::IpBan& ban);
    static bool Save(const AB::Entities::IpBan& ban);
    static bool Delete(const AB::Entities::IpBan& ban);
};

}
