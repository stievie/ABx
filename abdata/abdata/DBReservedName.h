#pragma once

#include <AB/Entities/ReservedName.h>

namespace DB {

class DBReservedName
{
public:
    DBReservedName() = delete;
    ~DBReservedName() = delete;

    static bool Create(AB::Entities::ReservedName&);
    static bool Load(AB::Entities::ReservedName& n);
    static bool Save(const AB::Entities::ReservedName&);
    static bool Delete(const AB::Entities::ReservedName&);
    static bool Exists(const AB::Entities::ReservedName& n);
};

}
