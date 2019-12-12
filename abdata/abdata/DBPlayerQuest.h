#pragma once

#include <AB/Entities/PlayerQuest.h>

namespace DB {

class DBPlayerQuest
{
public:
    DBPlayerQuest() = delete;
    ~DBPlayerQuest() = delete;

    static bool Create(AB::Entities::PlayerQuest& g);
    static bool Load(AB::Entities::PlayerQuest& g);
    static bool Save(const AB::Entities::PlayerQuest& g);
    static bool Delete(const AB::Entities::PlayerQuest& g);
    static bool Exists(const AB::Entities::PlayerQuest& g);
};

}
