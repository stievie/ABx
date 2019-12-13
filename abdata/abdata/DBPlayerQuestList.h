#pragma once

#include <AB/Entities/PlayerQuestList.h>

namespace DB {

class DBPlayerQuestList
{
public:
    DBPlayerQuestList() = delete;
    ~DBPlayerQuestList() = delete;

    static bool Create(AB::Entities::PlayerQuestList&);
    static bool Load(AB::Entities::PlayerQuestList&);
    static bool Save(const AB::Entities::PlayerQuestList&);
    static bool Delete(const AB::Entities::PlayerQuestList&);
    static bool Exists(const AB::Entities::PlayerQuestList&);
};

}
