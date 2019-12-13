#pragma once

#include <AB/Entities/PlayerQuest.h>

namespace DB {

class DBPlayerQuest
{
public:
    DBPlayerQuest() = delete;
    ~DBPlayerQuest() = delete;

    static bool Create(AB::Entities::PlayerQuest&);
    static bool Load(AB::Entities::PlayerQuest&);
    static bool Save(const AB::Entities::PlayerQuest&);
    static bool Delete(const AB::Entities::PlayerQuest&);
    static bool Exists(const AB::Entities::PlayerQuest&);
};

}
