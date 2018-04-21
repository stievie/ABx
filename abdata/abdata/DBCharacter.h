#pragma once

#include <AB/Entities/Character.h>

namespace DB {

class DBCharacter
{
public:
    DBCharacter() = delete;
    ~DBCharacter() = delete;

    static bool Create(AB::Entities::Character& character);
    static bool Load(AB::Entities::Character& character);
    static bool Save(const AB::Entities::Character& character);
    static bool Delete(const AB::Entities::Character& character);
};

}
