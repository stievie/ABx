#pragma once

#include <AB/Entities/Game.h>

namespace DB {

class DBGame
{
public:
    DBGame() = delete;
    ~DBGame() = delete;

    static bool Create(AB::Entities::Game&);
    static bool Load(AB::Entities::Game& game);
    static bool Save(const AB::Entities::Game&);
    static bool Delete(const AB::Entities::Game&);
    static bool Exists(const AB::Entities::Game& game);
};

}
