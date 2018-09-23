#pragma once

#include <AB/Entities/VersionList.h>

namespace DB {

class DBVersionList
{
public:
    DBVersionList() = delete;
    ~DBVersionList() = delete;

    static bool Create(AB::Entities::VersionList&);
    static bool Load(AB::Entities::VersionList& el);
    static bool Save(const AB::Entities::VersionList&);
    static bool Delete(const AB::Entities::VersionList&);
    static bool Exists(const AB::Entities::VersionList&);
};

}
