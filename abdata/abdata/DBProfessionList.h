#pragma once

#include <AB/Entities/ProfessionList.h>

namespace DB {

class DBProfessionList
{
public:
    DBProfessionList() = delete;
    ~DBProfessionList() = delete;

    static bool Create(AB::Entities::ProfessionList&);
    static bool Load(AB::Entities::ProfessionList& pl);
    static bool Save(const AB::Entities::ProfessionList&);
    static bool Delete(const AB::Entities::ProfessionList&);
    static bool Exists(const AB::Entities::ProfessionList&);
};

}
