#pragma once

#include <AB/Entities/AttributeList.h>

namespace DB {

class DBAttributeList
{
public:
    DBAttributeList() = delete;
    ~DBAttributeList() = delete;

    static bool Create(AB::Entities::AttributeList&);
    static bool Load(AB::Entities::AttributeList& al);
    static bool Save(const AB::Entities::AttributeList&);
    static bool Delete(const AB::Entities::AttributeList&);
    static bool Exists(const AB::Entities::AttributeList&);
};

}
