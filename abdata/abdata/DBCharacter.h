#pragma once

#include <AB/Entities/Character.h>

namespace DB {

class DBCharacter
{
public:
    DBCharacter() = delete;
    ~DBCharacter() = delete;

    static bool Load(AB::Entities::Character& account);
    static bool Save(AB::Entities::Character& account);
    static bool Delete(AB::Entities::Character& account);
};

}
