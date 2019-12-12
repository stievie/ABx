#pragma once

#include <AB/Entities/Quest.h>

namespace DB {

class DBQuest
{
public:
    DBQuest() = delete;
    ~DBQuest() = delete;

    static bool Create(AB::Entities::Quest& v);
    static bool Load(AB::Entities::Quest& g);
    static bool Save(const AB::Entities::Quest& g);
    static bool Delete(const AB::Entities::Quest& g);
    static bool Exists(const AB::Entities::Quest& g);
};

}
