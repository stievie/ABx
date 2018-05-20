#pragma once

#include <AB/Entities/Version.h>

namespace DB {

class DBVersion
{
public:
    DBVersion() = delete;
    ~DBVersion() = delete;

    static bool Create(AB::Entities::Version&);
    static bool Load(AB::Entities::Version& v);
    static bool Save(const AB::Entities::Version&);
    static bool Delete(const AB::Entities::Version&);
    static bool Exists(const AB::Entities::Version& v);
};

}
