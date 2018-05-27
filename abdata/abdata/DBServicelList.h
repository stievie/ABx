#pragma once

#include <AB/Entities/ServiceList.h>

namespace DB {

class DBServicelList
{
public:
    DBServicelList() = delete;
    ~DBServicelList() = delete;

    static bool Create(AB::Entities::ServiceList&);
    static bool Load(AB::Entities::ServiceList& sl);
    static bool Save(const AB::Entities::ServiceList&);
    static bool Delete(const AB::Entities::ServiceList&);
    static bool Exists(const AB::Entities::ServiceList&);
};

}
