#pragma once

#include <AB/Entities/AccountKey.h>

namespace DB {

class DBAccountKey
{
public:
    DBAccountKey() = delete;
    ~DBAccountKey() = delete;

    static bool Create(AB::Entities::AccountKey&);
    static bool Load(AB::Entities::AccountKey& ak);
    static bool Save(const AB::Entities::AccountKey& ak);
    static bool Delete(const AB::Entities::AccountKey&);
    static bool Exists(const AB::Entities::AccountKey& ak);
};

}
