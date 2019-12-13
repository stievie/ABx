#pragma once

#include <AB/Entities/Quest.h>

namespace DB {

class DBQuest
{
public:
    DBQuest() = delete;
    ~DBQuest() = delete;

    static bool Create(AB::Entities::Quest&);
    static bool Load(AB::Entities::Quest&);
    static bool Save(const AB::Entities::Quest&);
    static bool Delete(const AB::Entities::Quest&);
    static bool Exists(const AB::Entities::Quest&);
};

}
