#pragma once

#include <AB/Entities/QuestList.h>

namespace DB {

class DBQuestList
{
public:
    DBQuestList() = delete;
    ~DBQuestList() = delete;

    static bool Create(AB::Entities::QuestList&);
    static bool Load(AB::Entities::QuestList& q);
    static bool Save(const AB::Entities::QuestList&);
    static bool Delete(const AB::Entities::QuestList&);
    static bool Exists(const AB::Entities::QuestList&);
};

}
