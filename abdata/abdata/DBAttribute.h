#pragma once

#include <AB/Entities/Attribute.h>

namespace DB {

class DBAttribute
{
public:
    DBAttribute() = delete;
    ~DBAttribute() = delete;

    static bool Create(AB::Entities::Attribute&);
    static bool Load(AB::Entities::Attribute& attr);
    static bool Save(const AB::Entities::Attribute&);
    static bool Delete(const AB::Entities::Attribute&);
    static bool Exists(const AB::Entities::Attribute& attr);
};

}
