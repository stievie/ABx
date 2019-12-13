#pragma once

#include <AB/Entities/PlayerQuestListRewarded.h>

namespace DB {

class DBPlayerQuestListRewarded
{
public:
    DBPlayerQuestListRewarded() = delete;
    ~DBPlayerQuestListRewarded() = delete;

    static bool Create(AB::Entities::PlayerQuestListRewarded&);
    static bool Load(AB::Entities::PlayerQuestListRewarded&);
    static bool Save(const AB::Entities::PlayerQuestListRewarded&);
    static bool Delete(const AB::Entities::PlayerQuestListRewarded&);
    static bool Exists(const AB::Entities::PlayerQuestListRewarded&);
};

}
