#pragma once

#include <AB/Entities/PlayerQuestList.h>

namespace DB {

class DBPlayerQuestList
{
public:
    DBPlayerQuestList() = delete;
    ~DBPlayerQuestList() = delete;

    static bool Create(AB::Entities::PlayerQuestList& g);
    static bool Load(AB::Entities::PlayerQuestList& g);
    static bool Save(const AB::Entities::PlayerQuestList& g);
    static bool Delete(const AB::Entities::PlayerQuestList& g);
    static bool Exists(const AB::Entities::PlayerQuestList& g);
};

}
