#pragma once

#include <AB/Entities/GameList.h>

namespace DB {

class DBGameList
{
public:
    DBGameList() = delete;
    ~DBGameList() = delete;

    static bool Create(AB::Entities::GameList&);
    static bool Load(AB::Entities::GameList& game);
    static bool Save(const AB::Entities::GameList&);
    static bool Delete(const AB::Entities::GameList&);
    static bool Exists(const AB::Entities::GameList&);
};

}
